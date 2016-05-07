
#ifndef _GNU_SOURCE
#define _GNU_SOURCE         // See feature_test_macros(7) 
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

#include <sys/sendfile.h>



#include <poll.h>

#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        } while(0)

/*
       #include <poll.h>

       int poll(struct pollfd *fds, nfds_t nfds, int timeout);

       #define _GNU_SOURCE         // See feature_test_macros(7) 
       #include <poll.h>

       int ppoll(struct pollfd *fds, nfds_t nfds,
               const struct timespec *timeout_ts, const sigset_t *sigmask);
	
	   struct pollfd {
	   int   fd;         // file descriptor 
	   short events;     // requested events 
	   short revents;    // returned events 
   };

*/
			
int main( int argc, char* argv[] )
{
	if( argc <= 2 )
	{
		printf( "usage: %s ip_address port_number\n", basename( argv[0] ) );
		return 1;
	}
	const char* ip = argv[1];
	int port = atoi( argv[2] );
	printf( "ip is %s and port is %d\n", ip, port );

	int ret = 0;
	struct sockaddr_in address;
	bzero( &address, sizeof( address ) );
	address.sin_family = AF_INET;
	inet_pton( AF_INET, ip, &address.sin_addr );
	address.sin_port = htons( port );

	int listenfd = socket( PF_INET, SOCK_STREAM, 0 );
	assert( listenfd >= 0 );
	
	int on = 1;
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
		ERR_EXIT("setsockopt");

    ret = bind( listenfd, ( struct sockaddr* )&address, sizeof( address ) );
	assert( ret != -1 );

	ret = listen( listenfd, 5 );
	assert( ret != -1 );

	struct sockaddr_in client_address;
    socklen_t client_addrlength = sizeof( client_address );
	int connfd = accept( listenfd, ( struct sockaddr* )&client_address, &client_addrlength );
	if ( connfd < 0 )
	{
		printf( "errno is: %d\n", errno );
		close( listenfd );
	}

	char remote_addr[INET_ADDRSTRLEN];
	printf( "connected with ip: %s and port: %d\n", inet_ntop( AF_INET, &client_address.sin_addr, remote_addr, INET_ADDRSTRLEN ), ntohs( client_address.sin_port ) );

	char buf[1024];
	struct pollfd polfd[2];
	polfd[0].fd = connfd;
	polfd[0].events = POLLIN;
	
	polfd[1].fd = connfd;
	polfd[1].events = POLLRDHUP;
	nfds_t num = 2;
	

	int nReuseAddr = 1;
	setsockopt( connfd, SOL_SOCKET, SO_OOBINLINE, &nReuseAddr, sizeof( nReuseAddr ) );
	
	bool Flage = true;
	while( Flage )
	{
		memset( buf, '\0', sizeof( buf ) );
		
		ret = poll(polfd, num, 3000);
		if ( ret == 0 )
		{
			printf("timeout 300 millionsecond\n");
		}
		else if ( ret == -1 )
		{
				printf( "poll failure\n" );
				break;
		}
		
		for (int i = 0; i < 2; ++i)
		{
			/*
			if (polfd[i].revents & POLLIN)
			{
				int sockfd = polfd[i].fd;
				ret = recv( connfd, buf, sizeof( buf )-1, MSG_DONTWAIT|MSG_PEEK);
				//¿Í»§¶Ë¹Ø±Õ
				if( ret == 0 || (ret < 0 && errno != EAGAIN) )
				{
					printf("the client %d exit....\n", polfd[i].fd);
					Flage = false;
					break;
				}
				
				ret = recv( connfd, buf, sizeof( buf )-1, 0);
				printf( "get %d bytes of normal data: %s\n", ret, buf );
						
				send(connfd, buf, sizeof(buf), 0);
			}
			else if ( polfd[i].revents & POLLRDHUP )
			{
				printf("POLLRDHUP the client %d exit....\n", polfd[i].fd);
				Flage = false;
				break;
			}*/
			if ( polfd[i].revents & POLLRDHUP )
			{
				printf("POLLRDHUP the client %d exit....\n", polfd[i].fd);
				Flage = false;
				break;
			}
			
		}	

	}

	close( connfd );
	close( listenfd );
	return 0;
}
