#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#define DEFAULT_PORT 6666

int main(int argc, char **argv)
{
    int serverfd, accpetfd;
    struct sockaddr_in my_addr;
    struct sockaddr_in their_addr;
    unsigned int sin_size, myport = 6666, lisnum = 10;

    if ((serverfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return -1;
    }
    printf("socket is ok!\n");

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(DEFAULT_PORT);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(my_addr.sin_zero), 0);
    if (bind(serverfd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        return -2;
    }
    printf("bind is ok!\n");

    if (listen(serverfd, lisnum) == -1) {
        perror("listen");
        return -3;
    }
    printf("listen is ok!\n");

    fd_set client_fdset;
    int maxsock;
    struct timeval tv;
    int client_sockfd[10];
    bzero((void*)client_sockfd, sizeof(client_sockfd));
    int conn_amount = 0;
    maxsock = serverfd;
    char buffer[1024];
    int ret = 0;
    while (1) {
        FD_ZERO(&client_fdset);
        FD_SET(serverfd, &client_fdset);
        tv.tv_sec = 30;
        tv.tv_usec = 0;
        for (int i = 0; i < 10; ++i) {
            if (client_sockfd[i] != 0)
                FD_SET(client_sockfd[i], &client_fdset);
        }

        ret = select(maxsock + 1, &client_fdset, NULL, NULL, &tv);
        if (ret < 0) {
            perror("select");
            break;
        }
        else if (ret == 0) {
            printf("timeout!\n");
            continue;
        }
        for (int i = 0; i < conn_amount; ++i) {
            if (FD_ISSET(client_sockfd[i], &client_fdset)) {
                printf("start recv from client[%d]:\n", i);
                ret = recv(client_sockfd[i], buffer, 1024, 0);
                if (ret <= 0) {
                    printf("client[%d] close\n", i);
                    close(client_sockfd[i]);
                    FD_CLR(client_sockfd[i], &client_fdset);
                    client_sockfd[i] = 0;
                }
                else
                    printf("recv from client[%d]: %s\n", i, buffer);
            }
        }

        if (FD_ISSET(serverfd, &client_fdset)) {
            struct sockaddr_in client_addr;
            size_t size = sizeof(struct sockaddr_in);
            int sock_client = accept(serverfd, (struct sockaddr*)(&client_addr), (unsigned int*)(&size));
            if (sock_client < 0) {
                perror("accept error!\n");
                continue;
            }
            if (conn_amount < 10) {
                printf("new connection client[%d]: %s:%d\n", conn_amount, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
				client_sockfd[conn_amount++] = sock_client;

                bzero(buffer, 1024);
                strcpy(buffer, "this is server! welcome!\n");
                send(sock_client, buffer, 1024, 0);

                bzero(buffer, sizeof(buffer));
                ret = recv(sock_client, buffer, 1024, 0);
                if (ret < 0) {
                    perror("recv error!\n");
                    close(serverfd);
                    return -1;
                }
                printf("recv: %s\n", buffer);
                if (sock_client > maxsock)
                    maxsock = sock_client;
            }
            else {
                printf("max connection!!!quit!\n");
                break;
            }
        }
    }

    for (int i = 0; i < 10; ++i) {
        if (client_sockfd[i] != 0)
            close(client_sockfd[i]);
    }
    close(serverfd);
    return 0;
}
