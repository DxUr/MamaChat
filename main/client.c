#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "net/unix/rudp.h"
#include "extra/types.h"



void client(char* p_ip, unsigned int p_port) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[MAX_BUFFER_SIZE];

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(p_port);
    if (inet_aton(p_ip, &server_addr.sin_addr) == 0) {
        fprintf(stderr, "Invalid address\n");
        exit(1);
    }
    CLS
    printf(NAME);

    while (1) {
        
        printf("\033[1m\033[93mClient$~ \033[1m\033[92m");
        getchar();
        scanf("%"STR(MAX_BUFFER_SIZE)"[^\n]",buffer);
        ssize_t sent_len = rudp_sendto(sockfd, buffer, strlen(buffer), 0,
                                  (struct sockaddr *)&server_addr, sizeof(server_addr));

        if (sent_len < 0) {
            ERR("MSG not sent.")
            continue;
        }
    }

    close(sockfd);
}
