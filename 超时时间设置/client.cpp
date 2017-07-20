/*************************************************************************
    > File Name: client.cpp
    > Author: Zhang Zhihang
    > Mail: zhang_zh1010@163.com 
    > Created Time: 2017年07月18日 星期二 10时15分39秒
 ************************************************************************/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
using namespace std;

#define MAXLINE 4096;

int main(int argc, char **argv)
{
	int sockfd, n;
	char recvline[4096], sendline[4096];
	struct sockaddr_in servaddr;

	if (argc != 2) {
		printf("usage: ./client <ipaddress\n");
		return 0;
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
		return 0;
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(6666);
	if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
		printf("inet_pton error for %s\n", argv[1]);
		return 0;
	}

	if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
		printf("connect error: %s(errno: %d)\n", strerror(errno), errno);
		return 0;
	}

	struct timeval stTimeValStruct;
	stTimeValStruct.tv_sec = 2;
	stTimeValStruct.tv_usec = 0;
	if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &stTimeValStruct, sizeof(stTimeValStruct))) {
		printf("setsockopt error: %s(errno: %d)\n", strerror(errno), errno);
		return 0;
	}
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &stTimeValStruct, sizeof(stTimeValStruct))) {
		printf("setsockopt error; %s(errno: %d)\n", strerror(errno), errno);
		return 0;
	}

	ssize_t writeLen;
	char sendMsg[10] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '\0'};
	int count = 0;
	writeLen = write(sockfd, sendMsg, sizeof(sendMsg));
	if (writeLen < 0) {
		printf("write error: %s(errno: %d)\n", strerror(errno), errno);
		close(sockfd);
		return 0;
	}
	else {
		printf("write sucess, writeLen: %d, sendMsg: %s\n", (int)writeLen, sendMsg);
	}

	char readMsg[10] = {0};
	int readLine = read(sockfd, readMsg, sizeof(readMsg));
	if (readLine < 0) {
		printf("read error:  %s(errno: %d)\n", strerror(errno), errno);
		close(sockfd);
		return 0;
	}
	else {
		readMsg[9] = '\0';
		printf("read sucess, readLine: %d, readMsg: %s\n", (int)readLine, readMsg);
	}
	close(sockfd);
	return 0;
}

