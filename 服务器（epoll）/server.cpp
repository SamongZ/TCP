#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>

#define IPADDRESS "127.0.0.1"
#define PORT 6666
#define MAXSIZE 1024
#define LISTENQ 5
#define FDSIZE 1024
#define EPOLLEVENTS 100

/* 函数声明 */
/* 套接字的创建和绑定 */
int socket_bind(const char* ip, int port);
/* 多路IO复用 */
void do_epoll(int listenfd);
/* 事件处理函数 */
void handle_events(int epollfd, struct epoll_event *evens, int num, int listen, char* buf);
/* 处理接收到的连接 */
void handle_accept(int epollfd, int listenfd);
/* 读处理 */
void do_read(int epollfd, int fd, char* buf);
/* 写处理 */
void do_write(int epollfd, int fd, char* buf);
/* 添加事件 */
void add_event(int epollfd, int fd, int state);
/* 删除事件 */
void delete_event(int epollfd, int fd, int state);
/* 修改事件 */
void modify_event(int epollfd, int fd, int state);

int main()
{
    int listenfd;
    listenfd = socket_bind(IPADDRESS, PORT);
    listen(listenfd, LISTENQ);
    do_epoll(listenfd);
    return 0;
}

int socket_bind(const char* ip, int port)
{
    int listenfd;
    struct sockaddr_in servaddr;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        perror("socket error: ");
        exit(1);
    }
	printf("socket is ok!\n");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &servaddr.sin_addr);
	//servaddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind error:");
        exit(1);
    }
	printf("listen is ok!\n");

    return listenfd;
}

void do_epoll(int listenfd)
{
    int epollfd;
    struct epoll_event events[EPOLLEVENTS];
    int ret;
    char buf[MAXSIZE];
    memset(buf, 0, MAXSIZE);

    epollfd = epoll_create(FDSIZE);   // 创建一个描述符epoll

    add_event(epollfd, listenfd, EPOLLIN);  // 添加监听描述符
    while (1) {
        ret = epoll_wait(epollfd, events, EPOLLEVENTS, -1);
        handle_events(epollfd, events, ret, listenfd, buf);
    }
    close(epollfd);
}

void handle_events(int epollfd, struct epoll_event *events, int num, int listenfd, char *buf)
{
    int i;
    int fd;
    for (i = 0; i < num; i++) {
        fd = events[i].data.fd;
        if ((fd == listenfd) && (events[i].events & EPOLLIN))  // 如果监听描述符可读，即有新的连接
            handle_accept(epollfd, listenfd);
        else if (events[i].events & EPOLLIN)   // 监听描述符以外的描述符可读
            do_read(epollfd, fd, buf);
        else if (events[i].events & EPOLLOUT)   // 监听描述符以外的描述符可写
            do_write(epollfd, fd, buf);
    }
}

void handle_accept(int epollfd, int listenfd)
{
    int clifd;
    struct sockaddr_in cliaddr;
    socklen_t cliaddrlen = sizeof(cliaddr);
    clifd = accept(listenfd, (struct sockaddr*)&cliaddr, &cliaddrlen);
    if (clifd == -1)
        perror("accept error: ");
    else {
        printf("accept a new client: %s:%d\n", inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
        add_event(epollfd, clifd, EPOLLIN);
    }
}

void do_read(int epollfd, int fd, char *buf)
{
    int nread;
	bzero(buf, sizeof(buf));
    nread = read(fd, buf, MAXSIZE);
    if (nread == -1) {
        perror("read error: ");
        close(fd);
        delete_event(epollfd, fd, EPOLLIN);
    }
    else if (nread == 0) {
        fprintf(stderr, "client closed.\n");
        close(fd);
        delete_event(epollfd, fd, EPOLLIN);
    }
    else {
        printf("read msg is: %s\n", buf);
        modify_event(epollfd, fd, EPOLLOUT);  // 修改此描述符为可写，对客户端进行回复
    }
}

void do_write(int epollfd, int fd, char *buf)
{
    int nwrite;
    memset(buf, 0, MAXSIZE);
    strcpy(buf, "server: i had received.\n");
    nwrite = write(fd, buf, strlen(buf));
    if (nwrite == -1) {
        perror("write error: ");
        close(fd);
        delete_event(epollfd, fd, EPOLLOUT);
    }
    else
        modify_event(epollfd, fd, EPOLLIN);   // 回复客户端之后继续监测是否可读
    memset(buf, 0, MAXSIZE);
}

void add_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

void delete_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}

void modify_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}
