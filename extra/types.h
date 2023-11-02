#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>

#define CLS printf("\033[2J");\
            printf("\033[H");

#define STR(x) __STRING(x)

#define NAME "\
███╗░░░███╗░█████╗░███╗░░░███╗░█████╗░  ░█████╗░██╗░░██╗░█████╗░████████╗\n\
████╗░████║██╔══██╗████╗░████║██╔══██╗  ██╔══██╗██║░░██║██╔══██╗╚══██╔══╝\n\
██╔████╔██║███████║██╔████╔██║███████║  ██║░░╚═╝███████║███████║░░░██║░░░\n\
██║╚██╔╝██║██╔══██║██║╚██╔╝██║██╔══██║  ██║░░██╗██╔══██║██╔══██║░░░██║░░░\n\
██║░╚═╝░██║██║░░██║██║░╚═╝░██║██║░░██║  ╚█████╔╝██║░░██║██║░░██║░░░██║░░░\n\
╚═╝░░░░░╚═╝╚═╝░░╚═╝╚═╝░░░░░╚═╝╚═╝░░╚═╝  ░╚════╝░╚═╝░░╚═╝╚═╝░░╚═╝░░░╚═╝░░░\n\n"




#ifdef DEBUG
#define ERR(MSG) printf("\033[1m\033[91mError: %s at %s:%d\033[0m\n", MSG, __FILE__, __LINE__);
#define OK(MSG) printf("\033[1m\033[92mOk: %s at %s:%d\033[0m\n", MSG, __FILE__, __LINE__);
#define WRN(MSG) printf("\033[1m\033[93mWarning: %s at %s:%d\033[0m\n", MSG, __FILE__, __LINE__);
#define INF(MSG) printf("\033[1m\033[94mInfo: %s at %s:%d\033[0m\n", MSG, __FILE__, __LINE__);
#define SIG(MSG) printf("\033[1m\033[95mSignal: %s at %s:%d\033[0m\n", MSG, __FILE__, __LINE__);
#define LINE() printf("\033[1m\033[96mLine at %s:%d\033[0m\n", __FILE__, __LINE__);
#define LN LINE()
#else
#define ERR(MSG) printf("\033[1m\033[91mError: %s\033[0m\n", MSG);
#define OK(MSG) printf("\033[1m\033[92mOk: %s\033[0m\n", MSG);
#define WRN(MSG) printf("\033[1m\033[93mWarning: %s\033[0m\n", MSG);
#define INF(MSG) printf("\033[1m\033[94mInfo: %s\033[0m\n", MSG);
#define SIG(MSG) printf("\033[1m\033[95mSignal: %s\033[0m\n", MSG);
#define LINE() ((void)NULL);
#define LN LINE()
#endif

#define PASS ((void)NULL);
#define MAX_BUFFER_SIZE 1024
#define CLIENTS_POOL_TYPE uint32_t
#define MAX_CLIENTS_COUNT 32
#define DELTA 200000 // DELTA TIME BETWEEN PACKETS
#define null NULL

// just for debug
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

typedef uint16_t port_t;
typedef char* string_t;
typedef void (*callable_t)();
typedef char* buffer_t;
typedef char byte_t;
typedef uint8_t err;

typedef enum {
    NONE,
    SERVER,
    CLIENT
}mode_e;

typedef enum{
    CONNECTED,
    DISCONNECTED,
    CLIENT_CONNECTED,
    CLIENT_DISCONNECTED,
    RECV_DATA,
    //the last one for dynamic sizing the array...
    SIGNALS
}signal_e;


typedef struct {
    uint8_t op;
    uint32_t s_uid;
    uint8_t id;
    char alias[8];
    byte_t data;
}MSG_HEADER;


enum ErrorCode{
    ERR_OK = (err)(0),
    ERR_CANNOT_CREATE_SOCKET,
    ERR_CANNOT_BIND_ADDR,
    ERR_CANNOT_CONNECT_TO_SERVER,
    ERR_CANNOT_SEND_DATA,
    ERR_CANNOT_RECV_DATA,
    ERR_UNVALID_MSG,
    ERR_UNKNOWN_MSG,
    ERR_EMPTY_MSG,
};
