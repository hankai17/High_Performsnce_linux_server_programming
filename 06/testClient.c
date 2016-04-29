#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define BUFFER_SIZE 1024

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
	
	struct sockaddr_in server_address;
	bzero( &server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	
	inet_pton( AF_INET, ip, &server_address.sin_addr);
	server_address.sin_port = htons( port );
	
	
	
	int sockfd = socket(PF_INET, SOCK_STREAM, 0);
	assert( sockfd >= 0);
	
	if( connect(sockfd, (struct sockaddr*)&server_address,
							sizeof(server_address)) < 0)
	{
		close(sockfd);
		ERR_EXIT("connetc failed\n");
	}
	else
	{
		int ret = 0;
		char buffer[BUFFER_SIZE];
		while( (ret = recv(sockfd, buffer, BUFFER_SIZE-1, 0)) > 0)
		{
			printf("got %d bytes buffer is '%s'\n", ret, buffer);
			memset(buffer, '\0', sizeof(buffer));
		}
	}
	
	close(sockfd);
	return 0;
	
}
