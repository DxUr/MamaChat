#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "net/unix/rudp.h"
#include "extra/types.h"




void server(unsigned int p_port) {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[MAX_BUFFER_SIZE];

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(p_port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(1);
    }
    CLS
    printf(NAME);
    fflush(stdout);
    while (1) {
        socklen_t client_len = sizeof(client_addr);

        ssize_t recv_len = rudp_recvfrom(sockfd, buffer, sizeof(buffer), 0,
                                    (struct sockaddr *)&client_addr, &client_len);

        if (recv_len == -1) {
            perror("recvfrom");
            exit(1);
        }

        buffer[recv_len] = '\0';

        printf("\033[1m\033[92mServer$~ \033[1m\033[95m%s\033[0m\n", buffer);
    }

    close(sockfd);
}