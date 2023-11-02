#include "server.h"
#include "rudp.h"



static uint32_t session_uid = 0;
static int sockfd;
static unsigned int clients_count = 0;
static struct sockaddr_in server_addr, clients_addr[MAX_CLIENTS_COUNT];
extern char alias[8];
// AS MAX CLIENTS 32 FOR JUST I'll do that for playing with values 
// bool is 1Bytes mean a pool of clients needs MAX_CLIENTS_COUNT Bytes
// with this way the pool needs only MAX_CLIENTS_COUNT Bits
static CLIENTS_POOL_TYPE clients_pool = 0;
static socklen_t client_addr_len = sizeof(struct sockaddr_in);


err start_server(port_t p_port) {

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        ERR("ERR_CANNOT_CREATE_SOCKET")
        return ERR_CANNOT_CREATE_SOCKET;
    }

    // Set server address details
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(p_port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind address to the socket
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        ERR("ERR_CANNOT_BIND_ADDR")
        return ERR_CANNOT_BIND_ADDR;
    }
    printf("UDP server is listening on port %d...\n", p_port);
    return ERR_OK;
}

static err _send_data(struct sockaddr_in* addr_in, buffer_t p_buff) {
    MSG_HEADER* msg = p_buff;
    msg->s_uid = session_uid;
    if (rudp_sendto(sockfd, p_buff, MAX_BUFFER_SIZE, 0, (struct sockaddr *)addr_in, client_addr_len) == -1) {
        return ERR_CANNOT_SEND_DATA;
    }
    return ERR_OK;
}

void server_dispatch_data(buffer_t p_buff) {
    MSG_HEADER* msg = p_buff;
    for (uint8_t i = 0; i < clients_count; i++)
    {
        if ((clients_pool >> i) & 0) return;
        err _err = _send_data(&clients_addr[i], p_buff);
        // TODO: error handeling here
        // or just set user to disconnected
        clients_pool |= (1u << i);
    }
}

err server_get_data(buffer_t p_buff) {
    // Receive data from clients
    // Must clear the buffer
    memset(p_buff, 0x0, sizeof(MAX_BUFFER_SIZE));
    struct sockaddr_in client_addr;
    ssize_t num_bytes = rudp_recvfrom(sockfd, p_buff, MAX_BUFFER_SIZE - 1, 0,
                                    (struct sockaddr *)&client_addr, &client_addr_len);
    if (num_bytes == -1) {
        return ERR_CANNOT_RECV_DATA;
    }
    p_buff[num_bytes] = 0x0;
    // Validate data, check if client is a new client
    // packet should have first byte as op then 4 bytes session uid
    // if op == 0 means client want to sent or ... if 1 means client look if there are a session here
    // so will return a session uid
    // then client id (int no more than 256 client)
    // if client id == 0 means new client
    MSG_HEADER* msg = p_buff;
    if (msg->op == 0 && msg->s_uid == session_uid && (clients_pool >> msg->id-1) & 1) {
        //we have valid client well the message will be dispatched
        server_dispatch_data(p_buff);
        return ERR_OK;
    } else if (msg->op == 1) {
        // means client wanna connect so first needs to regester the client
        bool reg_new = false;
        int reg_id = clients_count + 1;
        if (msg->id) PASS
        else if(clients_count < MAX_CLIENTS_COUNT) {
            reg_new = true;
        } else if (clients_pool != UINT32_MAX) {
            for (int i = 0; i < 32; i++) {
                if ((clients_pool >> i) & 1) {
                    reg_new = true;
                }
            }
        }
        if (reg_new) {
            printf("\033[2K\r");
            printf("\033[1m\033[92m(\033[1m\033[96m%s\033[1m\033[92m) Connected\033[0m\n\n", msg->alias);
            printf("\033[1m\033[92m(\033[1m\033[93m%s\033[1m\033[92m)$ \033[0m", alias);
            fflush(stdout);
            clients_addr[clients_count] = client_addr;
            clients_count++;
            msg->s_uid = session_uid;
            msg->id = reg_id;
            p_buff[sizeof(MSG_HEADER)] = 0x0; // I'll change this cus it does not make sens as i use the strend as messege end ...
            server_dispatch_data(p_buff);
            msg->op = 2;
            _send_data(&client_addr, p_buff);
        }
    } else {
        return ERR_UNVALID_MSG;
    }
    return ERR_OK;
}

void close_server()
{
    close(sockfd);
}
