#include "chat.h"
#include "net/net.h"



extern mode_e current_mode;
extern char alias[8];
extern int id;

int strempty(const string_t p_str) {
    char* str = p_str;
    while (*str && *str == ' ') {
        str++;
    }

    return (*str == '\0');
}

void connected() {
    CLS
    byte_t buffer[MAX_BUFFER_SIZE];
    while (1)
    {
        memset(buffer, 0x0, MAX_BUFFER_SIZE);
        MSG_HEADER* msg = buffer;
        msg->op = 0;
        msg->id = id;
        strncpy(&(msg->alias), alias, sizeof(alias));
        printf("\033[1m\033[92m(\033[1m\033[93m%s\033[1m\033[92m)$\033[0m", alias);
        fflush(stdout);
        getchar();
        scanf("%[^\n]", &(msg->data));
        printf("\n");
        if(strempty(&(msg->data))) continue;
        Net_Handler.send_data(buffer);
    }

}

void disconnected() {
    SIG("DISCONNECTED")
    exit(0);
}

void client_connected(buffer_t p_buff) {
    INF("Someone get here")
}

void client_disconnected(buffer_t p_buff) {
    INF("Someone get out")
}

void recv_data(buffer_t p_buff) {
    MSG_HEADER* msg = p_buff;
    if(strempty(&msg->data)) return;
    printf("\033[2K\r");
    printf("\033[1m\033[92m(\033[1m\033[96m%s\033[1m\033[92m)$\033[1m\033[95m%s\033[0m\n\n", msg->alias, &msg->data);
    printf("\033[1m\033[92m(\033[1m\033[93m%s\033[1m\033[92m)$\033[0m", alias);
    fflush(stdout);
}