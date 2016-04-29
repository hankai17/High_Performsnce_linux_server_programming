#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>

#include <libgen.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        } while(0)


int main( int argc, char* argv[] )
{
	/*
    if( argc <= 2 )
    {
        printf( "usage: %s ip_address port_number\n", basename( argv[0] ) );
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi( argv[2] );
	
	printf("%s, %d\n", ip, port);
	*/
    int sock = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
	assert( sock >= 0 );
	
    struct sockaddr_in address;
    bzero( &address, sizeof( address ) );
    address.sin_family = AF_INET;
    //inet_pton( AF_INET, ip, &address.sin_addr );
	address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons( 8001 );


	
	int optval = 1;
	int rec = 0;
	rec = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	assert( rec != -1 );

    int ret = bind( sock, ( struct sockaddr* )&address, sizeof(address) );
    assert( ret != -1 );

    ret = listen( sock, SOMAXCONN);
    assert( ret != -1 );	

	
	int connfd; 
    struct sockaddr_in client;
    socklen_t client_addrlength = sizeof(client);
	
	while (1)
	{
		if ((connfd = accept(sock, (struct sockaddr*)&client, &client_addrlength)) < 0)
		{
			close(sock);
			close(connfd);
			ERR_EXIT("accept");
		}
			

		printf("ip=%s port=%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));


		//close(connfd);  //注意1 

	}

    close(sock);
    return 0;
}



