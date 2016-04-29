#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        } while(0)

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("Usage: %s ip_adddress port_number \n", basename( argv[0]) );
		return 1;
	}
	
	const char *ip = argv[1];
	int port = atoi(argv[2]);
	
	struct sockaddr_in address;
	bzero( &address, sizeof(address));
	address.sin_family = AF_INET;
	
	inet_pton( AF_INET, ip, &address.sin_addr);
	address.sin_port = htons( port );
	
	
	int sockfd = socket(PF_INET, SOCK_STREAM, 0);
	assert( sockfd >= 0);
	
	int on = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
		ERR_EXIT("setsockopt");
	
	int ret = bind(sockfd, (struct sockaddr*)&address, sizeof(address));
	assert( ret != -1);
	
	ret = listen(sockfd, 5);
	assert( ret != -1);
	
	struct sockaddr_in client;
	bzero(&client, sizeof(client));
	socklen_t client_length = sizeof(client);
	
	int connfd = accept( sockfd, (struct sockaddr*)&client, &client_length);
	if( connfd < 0)
	{
		close(sockfd);
		ERR_EXIT("accept ERROR: ");		
	}
	else
	{
		close( STDOUT_FILENO);
		dup( connfd);
		printf("hello world");
		close(connfd);
	}
	close(sockfd);
	return 0;
}