#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <poll.h>
#include <sys/wait.h>
#include <string.h>

#define DEFAULT_PORT 6666
#define MAXLINE 1024
#define LISTENQ 5
#define OPEN_MAX 1000
#define INFTIM -1

int bind_and_listen()
{
    int serverfd;
    struct sockaddr_in my_addr;
    unsigned int sin_size;
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

    if (listen(serverfd, LISTENQ) == -1) {
        perror("listen");
        return -3;
    }
    printf("listen is ok!\n");

    return serverfd;
}

void do_poll(int listenfd)
{
    int connfd, sockfd;
    struct sockaddr_in client_addr;
    socklen_t cliaddrlen;
    struct pollfd clientfds[OPEN_MAX];
    int maxi;
    int i;
    int nready;
    /* 添加监听描述符 */
    clientfds[0].fd = listenfd;
    clientfds[0].events = POLLIN;
    /* 初始化客户连接描述符 */
    for (i = 1; i < OPEN_MAX; ++i)
        clientfds[i].fd = -1;
    maxi = 0;

    while (1) {
        /* 获取可用描述符个数 */
        nready = poll(clientfds, maxi + 1, INFTIM);
        if (nready == -1) {
            perror("poll");
            exit(1);
        }
        /* 检测监听描述符是否准备好 */
        if (clientfds[0].revents & POLLIN) {
            cliaddrlen = sizeof(client_addr);
            /* 接收新的连接 */
            if ((connfd = accept(listenfd, (struct sockaddr*)&client_addr, &cliaddrlen)) == -1) { 
				if (errno == EINTR)
                    continue;
                else {
                    perror("accept");
                    exit(1);
                }
            }
            /* 将新的连接添加到数组末尾 */
            for (i = 1; i < OPEN_MAX; ++i) {
				if (clientfds[i].fd < 0) {
					clientfds[i].fd = connfd;
                    break;
                }
            }
            if (i == OPEN_MAX) { 
				fprintf(stderr, "too many clients!\n");
                exit(1);
            }
            write(clientfds[i].fd, "this is server.\n", 16);
            fprintf(stdout, "accept a new client[%d]: %s:%d\n", i, inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
            clientfds[i].events = POLLIN;
            maxi = (i > maxi) ? i : maxi;
            if (--nready <= 0)
                continue;
        }
        /* 处理多个连接客户端发来的包 */
        char buffer[MAXLINE];
        memset(buffer, 0, MAXLINE);
        int readlen = 0;
        for (i = 1; i <= maxi; ++i) {
            if (clientfds[i].fd < 0)
                continue;
            /* 接收客户端发来的包 */
            if (clientfds[i].revents & POLLIN) { 
				readlen = read(clientfds[i].fd, buffer, MAXLINE);
                if (readlen == 0) {
					fprintf(stdout, "client[%d] is closed.\n", i);
                    close(clientfds[i].fd);
                    clientfds[i].fd = -1;
                    continue;
                }
                printf("recv from client[%d]: %s", i, buffer);
                
                /* 向客户端发送信息 */
				bzero(buffer, sizeof(buffer));
				strcpy(buffer, "server: i have received your msg.\n");
                write(clientfds[i].fd, buffer, MAXLINE);
            }
        }
    }
    for (int i = 0; i < LISTENQ; ++i) {
        if (clientfds[i].fd != -1)
            close(clientfds[i].fd);
    }
}

int main(int argc, char** argv)
{
    int listenfd = bind_and_listen();
    if (listenfd < 0)
        return 0;
    do_poll(listenfd);
    close(listenfd);
    return 0;
}
