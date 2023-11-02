#include "client.h"
#include "rudp.h"

static uint32_t session_uid = 0;
static int sockfd;
static struct sockaddr_in server_addr;
static unsigned int peers_count;
extern char alias[8];
extern int id;

err client_dispatch_data(buffer_t p_buff) {
    MSG_HEADER* msg = p_buff;
    msg->s_uid = session_uid;
    msg->id = id;
    strncpy(msg->alias, alias, sizeof(msg->alias) - 1);
    if (rudp_sendto(sockfd, p_buff, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        return ERR_CANNOT_SEND_DATA;
        // TODO: server disconected handle ERROR
    }
}

err start_client(string_t p_addr, port_t p_port) {

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        ERR("ERR_CANNOT_CREATE_SOCKET")
        return ERR_CANNOT_CREATE_SOCKET;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(p_port);
    if (inet_pton(AF_INET, p_addr, &(server_addr.sin_addr)) <= 0) {
        ERR("ERR_CANNOT_CONNECT_TO_SERVER")
        return ERR_CANNOT_CONNECT_TO_SERVER;
    }

    byte_t buff[MAX_BUFFER_SIZE];
    
    MSG_HEADER* msg = buff;
    msg->op = 1;
    msg->s_uid = 0;
    msg->id = 0;
    strncpy(msg->alias, alias, sizeof(msg->alias) - 1);

    client_dispatch_data(buff);
    

    printf("UDP client is connected.\n");
    return ERR_OK;
}

err client_get_data(buffer_t p_buff) {
    // Receive data from server
    // Must clear the buffer
    memset(p_buff, 0x0, sizeof(MAX_BUFFER_SIZE));
        ssize_t num_bytes = rudp_recvfrom(sockfd, p_buff, MAX_BUFFER_SIZE - 1, 0, NULL, NULL);
    if (num_bytes == -1) {
        return ERR_CANNOT_RECV_DATA;
    }
    p_buff[num_bytes] = 0x0;

    // TODO: Validate data:
    MSG_HEADER* msg = p_buff;
    if (msg->s_uid != session_uid) {
        return ERR_UNKNOWN_MSG;
    } else if (msg->op == 0) {
        if(msg->id != id) return ERR_OK;
        return ERR_EMPTY_MSG;
    } else if (msg->op == 1) {
        if(!id || msg->id == id) return ERR_OK;
        printf("\033[2K\r");
        printf("\033[1m\033[92m(\033[1m\033[96m%s\033[1m\033[92m) Connected\033[0m\n\n", msg->alias);
        printf("\033[1m\033[92m(\033[1m\033[93m%s\033[1m\033[92m)$ \033[0m", alias);
        fflush(stdout);
        return ERR_OK;
    } else if (msg->op == 2) {
        id = msg->id;
        return ERR_OK;
    }
    return ERR_UNVALID_MSG;
}

void close_client() {
    close(sockfd);
}