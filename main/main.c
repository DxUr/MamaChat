//////////////////
//      UDP
//


#include <stdio.h>
#include "extra/types.h"
#include "net/net.h"
#include "chat.h"


char alias[8] = "ME";
int id = 0;



// just for formating ipv4
static void strtrim(string_t str) {
    int index, i;
    index = 0;
    while(str[index] == ' ' || str[index] == '\t' || str[index] == '\n') index++;
    i = 0;
    while(str[i + index] != '\0') {
        str[i] = str[i + index];
        i++;
    }
    str[i] = '\0';
    i = 0;
    index = -1;
    while(str[i] != '\0') {
        if(str[i] != ' ' && str[i] != '\t' && str[i] != '\n') index = i;
        i++;
    }
    str[index + 1] = '\0';
}


int main(int argc, char const *argv[])
{
    CLS
    printf("enter alias:");
    scanf("%7[^\n]%*c", &alias);
    //ask user for the mode client/server
    int mode;
    printf("select mode: (%d)server, (%d)client, (otherwise)quit:", SERVER, CLIENT);
    scanf("%d", &mode);
    if (mode == SERVER) {
        Net_Handler.connect_sig(CONNECTED, connected);
        Net_Handler.connect_sig(DISCONNECTED, disconnected);
        Net_Handler.connect_sig(CLIENT_CONNECTED, client_connected);
        Net_Handler.connect_sig(CLIENT_DISCONNECTED, client_disconnected);
        Net_Handler.connect_sig(RECV_DATA, recv_data);
        port_t port;
        printf("Enter listening port:");
        scanf("%d", &port);
        INF("Starting server")
        Net_Handler.create_server(port);
    } else if (mode == CLIENT)
    {
        Net_Handler.connect_sig(CONNECTED, connected);
        Net_Handler.connect_sig(DISCONNECTED, disconnected);
        Net_Handler.connect_sig(CLIENT_CONNECTED, client_connected);
        Net_Handler.connect_sig(CLIENT_DISCONNECTED, client_disconnected);
        Net_Handler.connect_sig(RECV_DATA, recv_data);
        int port;
        char addr[100];
        printf("Enter address and port (address:port): ");
        scanf("%99[^:]:%d", addr, &port);
        strtrim(addr);
        INF("Starting client")
        Net_Handler.connect_to(addr, (port_t)port);
    } else {
        INF("Quiting...")
        return 0;
    }
    return 0;
}
