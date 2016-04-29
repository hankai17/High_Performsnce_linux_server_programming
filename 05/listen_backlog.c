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
	printf("I am going out...\n");
}

int main(int argc, char* argv[])
{
	signal(SIGTERM, handle_term);
	
	if(argc <= 3)
	{
		printf("Usage: %s ip_address port_number backlog\n",
				basename(argv[0]));
		return 1;
	}
	
	const char* ip = argv[1];
	int port = atoi(argv[2]);
	int backlog = atoi(argv[3]);
	
	int sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		perror("socket error:");
		return -1;
	}
	/*
	           struct sockaddr_in {
               sa_family_t    sin_family; // address family: AF_INET 
               in_port_t      sin_port;   // port in network byte order 
               struct in_addr sin_addr;   // internet address 
           };

	*/
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	inet_pton(AF_INET, ip, &address.sin_addr);
	
	int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
	if (ret < 0)
	{
		perror("bind error:");
		return -3;
	}
	
	ret = listen(sock, backlog);
	if (ret < 0)
	{
		perror("listen error:");
		return -4;
	}
	
	while( !stop)
	{
		sleep(1);
	}
	
	close(sock);
	return 0;
}