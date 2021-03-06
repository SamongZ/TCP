/*************************************************************************
    > File Name: server.cpp
    > Author: Zhang Zhihang
    > Mail: zhang_zh1010@163.com 
    > Created Time: 2017年07月18日 星期二 09时56分41秒
 ************************************************************************/

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
using namespace std;

#define MAXLINE 4096

int main()
{
	int listenfd, connfd;
	struct sockaddr_in servaddr;
	char buff[4096];
	int n;

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
		return 0;
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(6666);

	if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
		printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
		return 0;
	}

	if (listen(listenfd, 10) == -1) {
		printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
		return 0;
	}

	printf("======waiiting for client's request======\n'");
	while (1) {
		if ((connfd = accept(listenfd, (struct sockaddr*)NULL, NULL)) == -1 ) {
			printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
			continue;
		}
		n = recv(connfd, buff, MAXLINE, 0);
		buff[n] = '\0';
		printf("recv msg from client: %s\n", buff);
		close(connfd);
	}
	close(listenfd);
	return 0;
}

