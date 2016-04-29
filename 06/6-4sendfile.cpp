#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/uio.h>

/*
	ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count);
*/

#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        } while(0)

int main(int argc, char *argv[])
{
    if( argc <= 3 )
    {
        printf( "usage: %s ip_address port_number filename\n", basename( argv[0] ) );
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi( argv[2] );
    const char* file_name = argv[3];
	
	int filefd = open( file_name, O_RDONLY);
	assert( filefd > 0 );
	struct stat file_stat;
	int fsta = fstat( filefd, &file_stat );
	
	

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
		bool valid = true;
        if( fsta < 0 )
        {
            valid = false;
        }
        else
        {
            if( S_ISDIR( file_stat.st_mode )  )
            {
                valid = false;
            }
            else if( !(file_stat.st_mode & S_IROTH) ) /*others have read permission 判断是否有读权限*/
            {
				valid = false;
            }
        }
        
        if( valid )
        {
			sendfile(connfd, filefd, NULL,file_stat.st_size);
        }
        close( connfd );
    }

    close( sock );
    return 0;

}