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
#include <fcntl.h>



#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        } while(0)

int main(int argc, char *argv[])
{
    if( argc <= 2 )
    {
        printf( "usage: %s ip_address port_number \n", basename( argv[0] ) );
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi( argv[2] );
	
    struct sockaddr_in address;
    bzero( &address, sizeof( address ) );
    address.sin_family = AF_INET;
    inet_pton( AF_INET, ip, &address.sin_addr );
    address.sin_port = htons( port );

    int sock = socket( PF_INET, SOCK_STREAM, 0 );
    assert( sock >= 0 );
	
	/*设置多路复用*/
	int on = 1;
	int ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	assert( ret != -1);
	
    ret = bind( sock, ( struct sockaddr* )&address, sizeof( address ) );
    assert( ret != -1 );

    ret = listen( sock, 5 );
    assert( ret != -1 );

    struct sockaddr_in client;
    socklen_t client_addrlength = sizeof( client );
    int connfd = accept( sock, ( struct sockaddr* )&client, &client_addrlength );
    if ( connfd < 0 )
    {
        printf( "errno is: %d\n", errno );
    }
    else
    {
		int pipefd[2];
		
		ret = pipe( pipefd );  /*创建管道*/
		assert(ret != -1);
		/*将 connfd上流入的客户端数据定向到管道中*/
		ret = splice(connfd, NULL, pipefd[1], NULL, 32768,
						SPLICE_F_MORE | SPLICE_F_MOVE);
		assert(ret != -1);
		
		/*将 管道的输出定向到connfd客户连接描述符  (回射)*/ 
		ret = splice(pipefd[0], NULL, connfd, NULL, 32768,
						SPLICE_F_MORE | SPLICE_F_MOVE);
		assert(ret != -1);
		close(pipefd[0]);
		close(pipefd[1]);
		
        close( connfd );
    }

    close( sock );
    return 0;

}