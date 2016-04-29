#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#include <libgen.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

static int  stop = 0;

static void handle_term(int sig)
{
	stop = 1;
	printf("%d I am going out....\n", sig);
}

int main(void)
{
	signal(SIGINT, handle_term);

	const char* ip = "127.0.0.1";
	int   port 	   = 8001;
	
	int sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		perror("socket error:");
		return -1;
	}	
	
	/*
	       int connect(int sockfd, const struct sockaddr *addr,
                   socklen_t addrlen);

	*/
    struct sockaddr_in address;
    bzero( &address, sizeof( address ));
    address.sin_family = AF_INET;
    inet_pton( AF_INET, ip, &address.sin_addr );
    address.sin_port = htons( port );	
	
	int ret = connect(sock, (struct sockaddr*)&address, sizeof(address));
	if (ret < 0)
	{
		close(sock);
		perror("socket error:");
		return -1;
	}

	pause();
	
	close(sock);
	return 0;
	
}