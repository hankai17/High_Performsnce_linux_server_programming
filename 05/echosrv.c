#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

//wait(NULL) 只要有一个子进程退出，那么wait就会返回
//若多个子进程，只能等待一个。

//while (waitpid(-1, NULL, WNOHANG) > 0)
//	;
//WNOHANG 不挂起，如果没有子进程，会返回-1，从而可以跳出循环，信号函数执行完毕

#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        } while(0)



int main(void)
{

	int listenfd;
	if ((listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
/*	if ((listenfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)*/
		ERR_EXIT("socket");

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(8001);
	//servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	/*inet_aton("127.0.0.1", &servaddr.sin_addr);*/

	int on = 1;
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
		ERR_EXIT("setsockopt");

	if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
		ERR_EXIT("bind");
		
	if (listen(listenfd, SOMAXCONN) < 0)
		ERR_EXIT("listen");

	struct sockaddr_in peeraddr;
	socklen_t peerlen = sizeof(peeraddr);
	int conn;

	while (1)
	{
		if ((conn = accept(listenfd, (struct sockaddr*)&peeraddr, &peerlen)) < 0)
		{
			close(listenfd);
			close(conn);
			ERR_EXIT("accept");
		}
			

		printf("ip=%s port=%d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));


		close(conn);  //注意1 

	}
	
	
	close(listenfd);
	return 0;
}
