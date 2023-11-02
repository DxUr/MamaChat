#include "rudp.h"
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <memory.h>
//temp
#include "extra/types.h"

// this is the protocol for rudp this just ov//ERRide sendto, recvfrom functions
// i will just ov//ERRide the functions idont have a time.


#define MAX_SEG_SIZE 1024


#define SYN 0b10000000
#define ACK 0b01000000
#define TER 0b00100000
#define RST 0b00010000
#define NUL 0b00001000
#define TCS 0b00000100
#define DAT 0b00000010

//Base of all headers...
typedef struct {
    uint8_t control_bits;
    uint8_t header_len;
    uint8_t seq_num;
    uint8_t ack_num;
    uint16_t conn_id;// this for provent condlit packets
} RUDP_H;

// DATA TRANS HEDER
typedef struct {
    RUDP_H _;
    uint16_t data_sz;
    uint16_t checksum;
} RDP_H;


typedef struct {
    RUDP_H _;
    uint16_t checksum;
} ACK_H;

// SYN HEADER
typedef struct {
    RUDP_H _;
    uint16_t max_seg_sz;
    uint16_t time_out;
    uint16_t data_sz;
    uint8_t max_out_seq;
    uint8_t max_retran;
    uint16_t c_i;
    uint16_t checksum;
} SYN_H;


void printHexBytes(const unsigned char* data, int length) {
    for (int i = 0; i < length; i++) {
        printf("%02x ", data[i]);
    }

    printf("\n");
}

unsigned short calc_checksum(unsigned char* data, int length) {
    unsigned short sum1 = 0;
    unsigned short sum2 = 0;
    int i;
    for (i = 0; i < length; i++) {
        sum1 = (sum1 + data[i]) % 255;
        sum2 = (sum2 + sum1) % 255;
    }

    unsigned short checksum = (sum2 << 8) | sum1;
    return checksum;
}



int is_data_not_valid(char* p_buff) {
    RUDP_H* h = p_buff;
    uint16_t* checksum = ((char*)h + h->header_len * sizeof(char));
    checksum--;
    uint16_t sum = *checksum;
    *checksum = 0x0;
    if(calc_checksum(p_buff, h->header_len) != sum) {
        return 1;
    }
    return 0;
}

void validate_data(char* p_buff) {
    RUDP_H* h = p_buff;
    uint16_t* checksum = ((char*)h + h->header_len * sizeof(char));
    checksum--;
    *checksum = 0x0;
    uint16_t sum = calc_checksum(p_buff, h->header_len);
    memcpy(checksum, &sum, sizeof(uint16_t));
}

ssize_t recvfrom_to(int __fd, void *__restrict __buf, size_t __n, int __flags,
__SOCKADDR_ARG __addr, socklen_t *__restrict __addr_len, int __usec) {
    //printf("start timer:%d\n", __usec);
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(__fd, &readfds);
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = __usec;
    int selectResult = select(__fd + 1, &readfds, NULL, NULL, &timeout);
    if (selectResult == -1) {
    //printf("return -3;\n");
        return -3;
    } else if (selectResult == 0) {
    //printf("return -2;\n");
        return -2;
    }
    //printf("return;\n");
    return recvfrom(__fd, __buf, __n, __flags, __addr, __addr_len);
}


ssize_t rudp_recvfrom(int __fd, void *__restrict __buf, size_t __n, int __flags,
__SOCKADDR_ARG __addr, socklen_t *__restrict __addr_len) {
    // well for this imple i will not neogiate with the sender

    //SIG("recv  using rudp")
    struct sockaddr client_addr;// = *__addr;
    socklen_t client_addr_len;// = *__addr_len;
    bool recieving = 0;
    bool complete = false;

    char* wbuff_pointer = __buf;
    uint8_t seq_num = 0;

    uint16_t data_sz;
    uint16_t max_seg_sz = MAX_SEG_SIZE;
    uint16_t time_out = 1000;
    uint8_t max_out_seq;
    uint8_t max_retran;
    uint8_t c_i;
    uint16_t conn_id;
    uint8_t retran_att = 0;

    char buff[MAX_SEG_SIZE];
    memset(buff, 0, sizeof(buff));
    while (1) {
        //SIG("recv loop")
        char _buff[MAX_SEG_SIZE];
        memset(_buff, 0, sizeof(buff));
        ssize_t num_bytes = recieving?recvfrom_to(__fd, _buff, MAX_SEG_SIZE - 1, 0, &client_addr, &client_addr_len, time_out * 1000) :
        recvfrom(__fd, _buff, MAX_SEG_SIZE - 1, 0, &client_addr, &client_addr_len);
        if (num_bytes < 0 && !recieving) continue;
        else {
            switch (num_bytes)
            {
            case -1:
                return -1;
                break;
            case -2:
                // resend last data;
                if (retran_att < max_retran) {
                    if (sendto(__fd, buff, sizeof(buff), __flags, &client_addr, client_addr_len) == -1) return -1;
                    retran_att ++;
                } else return -1;
                continue;
                break;
            case -3:
                return -1;
                break;
            }
        }

        if (is_data_not_valid(_buff)) {
            if (!recieving) continue;
            // resend last data;
            if (retran_att < max_retran) {
                if (sendto(__fd, buff, sizeof(buff), __flags, &client_addr, client_addr_len) == -1) return -1;
                retran_att ++;
            } else return -1;
            continue;
        }


        RUDP_H* h = _buff;
        uint8_t _ack = h->ack_num, _seq = h->seq_num;
        if (h->control_bits == NULL) continue;
        if (!recieving) {
        //INF("here")
            // check if it a sync packet ... if no 
            if (h->control_bits == SYN) {
                //INF("Get syn.")
                seq_num ++;
                SYN_H* syn = h;
                if (syn->max_seg_sz > max_seg_sz) syn->max_seg_sz = max_seg_sz;
                else max_seg_sz = syn->max_seg_sz;
                time_out = syn->time_out;
                data_sz = syn->data_sz;
                conn_id = h->conn_id;
                max_out_seq = syn->max_out_seq;
                max_retran = syn->max_retran;
                c_i = syn->c_i;

                h->control_bits = ACK | SYN;
                    h->conn_id = conn_id;
                h->seq_num = seq_num;
                h->ack_num = _seq;

                //printf("control:%d, seq number:%d, _ack:%d\n",h->control_bits, seq_num, h->ack_num);
                validate_data(syn);
                //printf("control:%d, seq number:%d, _ack:%d\n",h->control_bits, seq_num, h->ack_num);
                if (sendto(__fd, syn, sizeof(SYN_H), __flags, &client_addr, client_addr_len) == -1) return -1;
                recieving = true;
                //INF("start recv")
            }
            continue;
        } else {
            if (h->conn_id != conn_id) {
                //ERR("drop data conn_id!")
                continue;
            } else //INF("Conn_ID")
            if ((h->control_bits & ACK) == ACK) {
        //INF("here")
        //printf("%d,, %d\n", seq_num, _ack);
                if (_ack == seq_num-1) {
                    //ok last packet, analys packet content then send next
                    //INF("move on");
                    if ((h->control_bits & SYN) == SYN) {
                        //INF("Get SYN back")
                        // means every thin ok from last setting not do any thing
                        // now send ACK packet well i will send ACK packet for all packets
                        // no copy data to buffer not do any thing for this impl
                    } 
                    else if ((h->control_bits & DAT) == DAT) {
                        // a data packet needs to be copied to buffer
                        //ERR("DAddddddddddddddddddddddd\n\n\nddddddddT")
                        RDP_H* rdp = h;
                        if ((char*)wbuff_pointer - (char*)__buf > data_sz)
                            continue; // well if over data;
                        if (rdp->data_sz > max_seg_sz - sizeof(RDP_H))
                            continue; // well this means the packet is broken the sender must respect the max data size
                        void* data = (char*)h + h->header_len;
                        memcpy(wbuff_pointer, data, rdp->data_sz);
                        wbuff_pointer += rdp->data_sz;
                    }
                    else if ((h->control_bits & TER) == TER) {
                        //ERR("TER")
                        // means data transfer complete
                        // I'll not send an TER packet I'll send ACK and return, for simplicity.
                        complete = true;
                    }
                    // clear the buffer
                    memset(buff, 0, sizeof(buff));
                    // send ACK
                    //WRN("send ACK")
                    ACK_H* ack = buff;
                    h = ack;
                    h->control_bits = ACK;
                    h->conn_id = conn_id;
                    h->seq_num = seq_num;
                    h->ack_num = _seq;
                    h->header_len = sizeof(ACK_H);
                    //printHexBytes(ack, sizeof(ACK_H));
                    validate_data(ack);
                    ////INF("ack................")
                    //printHexBytes(ack, sizeof(ACK_H));
                    if (sendto(__fd, buff, sizeof(buff), __flags, &client_addr, client_addr_len) == -1) return -1;
                    retran_att = 0;
                    seq_num ++;
                    // return bytes count
                    if (complete) {
                        //printf("data::%d", data_sz);
                        //printHexBytes(__buf, data_sz);

                        return data_sz;
                    }
                } if (_ack < seq_num-1) continue; // old packet.
                // if no i'll resend last packet as the sender should respect the sequence send
                //I should send an EAK but I'll send the last packet to the sender to correct the squence.
                else if (retran_att < max_retran) {
                    if (sendto(__fd, buff, sizeof(buff), __flags, &client_addr, client_addr_len) == -1) return -1;
                    retran_att ++;
                } else return -1;
            }
        }
    }
}

ssize_t rudp_sendto (int __fd, const void *__buf, size_t __n,int __flags,
__CONST_SOCKADDR_ARG __addr,socklen_t __addr_len) {
    //printHexBytes(__buf, __n);
    //SIG("send using rudp")

    struct sockaddr client_addr;// = *__addr;
    socklen_t client_addr_len;// = *__addr_len;

    char* rbuff_pointer = __buf;

    bool complete = false;

    uint16_t max_seg_sz = 256;
    uint16_t time_out = 1000;
    uint16_t data_sz = __n;
    uint8_t max_out_seq = 2;
    uint8_t max_retran = 4;
    uint16_t c_i = rand();
    uint16_t conn_id = rand();
    uint8_t retran_att = 0;
    

    srand(time());
    // sending data
    uint8_t seq_num = 0;
    // first send syn packet
    // initialize the susnc packet

    char buff[MAX_SEG_SIZE];
    memset(buff, 0, sizeof(buff));

    SYN_H* syn = buff;
    *syn = (SYN_H){
        ._ = (RUDP_H) {
            .control_bits = SYN,
            .header_len = sizeof(SYN_H),
            .seq_num = seq_num,
            .conn_id = conn_id,
        },
        .max_seg_sz = max_seg_sz,
        .time_out = time_out,
        .data_sz = data_sz,
        .max_out_seq = max_out_seq,
        .max_retran = max_retran,
        .c_i = c_i,
    };


    validate_data(syn);
    sendto(__fd, buff, sizeof(SYN_H), __flags, __addr, __addr_len);




    seq_num ++;

    while (1) {
        //SIG("send loop")
        char _buff[MAX_SEG_SIZE];
        memset(buff, 0, sizeof(_buff));
        //INF("get")
        ssize_t num_bytes = recvfrom_to(__fd, _buff, MAX_SEG_SIZE - 1, 0, &client_addr, &client_addr_len, time_out * 1000);
        //INF("ok")
        if (num_bytes < 0) {
            if (retran_att < max_retran) {
            retran_att ++;
            //SIG("continue")
            continue;
            }
            //SIG("return -1")
            return -1;
        }
        else {
            switch (num_bytes) {
            case -1:
                return -1;
                break;
            
            case -2:
            
                if (retran_att < max_retran) {
                    if (sendto(__fd, buff, sizeof(buff), __flags, __addr, __addr_len) == -1) return -1;
                    retran_att ++;
                } else return -1;
                continue;
                break;
            case -3:
                return -1;
                break;
            }
        }
        if (is_data_not_valid(_buff)) {
            // resend last data;
            //ERR("NOT VALID")
            if (retran_att < max_retran) {
                if (sendto(__fd, buff, sizeof(buff), __flags, __addr, __addr_len) == -1) return -1;
                retran_att ++;
            } else return -1;
            continue;
        }
        memcpy(buff, _buff, sizeof(_buff));
        RUDP_H* h = buff;
        if (h->conn_id != conn_id) {
            //printf("conn_id:%d, hconn_id:%d\n", conn_id, h->conn_id);
            //ERR("drop data conn_id!")
            continue;
        } // else INF("Conn_ID")
        uint8_t _ack = h->ack_num, _seq = h->seq_num;
        
        //WRN("RCV VALID")
        //printHexBytes(_buff, sizeof(RUDP_H));
        //printf("control:%02x, seq number:%d, _ack:%d\n",(h->control_bits), seq_num, h->ack_num);
        if ((h->control_bits & ACK) == ACK) {
            if (_ack == seq_num-1) {     
                //WRN("SEQ VALID")
                retran_att = 0;
                if (complete) {
                    // this is the last packet, then need to close
                    return 0;
                }

                // here we are
                // all packets must be ACK for flow control then I don't want to make it complicated
                // it's complicated enought
                if ((h->control_bits & SYN) == SYN) {  
                    //WRN("SYN VALID")
                    // get the SUN packet back from the reciever so setup the window...
                    SYN_H* syn = h;
                    h->conn_id = conn_id;
                    max_seg_sz = syn->max_seg_sz;
                    time_out = syn->time_out;
                    // data_sz = syn->data_sz; no need for that
                    max_out_seq = syn->max_out_seq;
                    max_retran = syn->max_retran;
                    validate_data(syn);
                    // then send it back to the reciever
                    if (sendto(__fd, buff, sizeof(buff), __flags, __addr, __addr_len) == -1) return -1;
                    //ERR("seq")
                    //printf("seq:%d\n", seq_num);
                    seq_num ++;
                }
                // then we need to start send data so
                // as a sender i dont need to send empty ACK the ACK must be a DATA packet
                else if ((rbuff_pointer - (char*)__buf) < __n) { // well for simplisity accept just ACK and SYN
                    // send the data to the reciever
                    memset(buff, 0, sizeof(buff));
                    // send DAT
                    RDP_H* dat = buff;
                    h = dat;
                    h->control_bits = DAT | ACK;
                    h->conn_id = conn_id;
                    h->seq_num = seq_num;
                    h->ack_num = _seq;
                    h->header_len = sizeof(RDP_H);

                    dat->data_sz = data_sz - (rbuff_pointer - (char*)__buf) > (max_seg_sz - h->header_len)?
                        (max_seg_sz - h->header_len):data_sz - (rbuff_pointer - (char*)__buf);

                    memcpy((char*)dat + h->header_len, rbuff_pointer, dat->data_sz);
                    //printf("datatsssssssssssssssssssssssssssss:%d", dat->data_sz);
                    rbuff_pointer += dat->data_sz;

                    //printf("header:%d, expected:%d\n", h->header_len);
                    validate_data(dat);
                    // send it;
                    //printf("control:%d, expected:%d\n", h->control_bits, DAT | ACK);
                    //printHexBytes(dat, 16);
                    if (sendto(__fd, buff, sizeof(buff), __flags, __addr, __addr_len) == -1) return -1;
                    //ERR("seq")
                    //printf("seq:%d\n", seq_num);
                    seq_num ++;
                } else {
                    // send TER packet.

                    memset(buff, 0, sizeof(buff));
                    // send TER (terminate)
                    ACK_H* ter = buff;
                    h->control_bits = TER | ACK;
                    h->conn_id = conn_id;
                    h->seq_num = seq_num;
                    h->ack_num = _seq;
                    h->header_len = sizeof(ACK_H);
                    validate_data(ter);
                    if (sendto(__fd, buff, sizeof(buff), __flags, __addr, __addr_len) == -1) return -1;
                    //ERR("TERMINATING \n\n\nTERMINAT")
                    //printf("seq:%d\n", seq_num);
                    seq_num ++;
                    
                    complete = true;
                }
                
            } else {
                // out of sequence
                // this //ERR will never ocure on my imple
                //ERR("out of sequence??u should not see this")
            }
        }



    }
    
}