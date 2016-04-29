#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#define BUFFER_SIZE 4096
/*
	主状态机：
	CHECK_STATE_REQUESTLINE： 当前正在分析请求行
	CHECK_STATE_HEADER	   ： 当前正在分析头部字段
*/
enum CHECK_STATE { CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER };
/*
	从状态机：
	LINE_OK  ：读取到一个完整行
	LINE_BAD ：行出错
	LINE_OPEN：行数据尚不完整
*/
enum LINE_STATUS { LINE_OK = 0, LINE_BAD, LINE_OPEN };
/*
	处理http请求结果：
	NO_REQUEST       ： 请求不完整，需要继续读取客户数据
	GET_REQUEST      ： 获得了完整的客户请求
	BAD_REQUEST      ： 请求语法错误
	INTERNAL_ERROR   ： 服务器内部错误
	CLOSED_CONNECTION： 客户端已经关闭连接
*/
enum HTTP_CODE { NO_REQUEST, GET_REQUEST, BAD_REQUEST, FORBIDDEN_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION };

/*
	服务器处理结果（简化）
	szret[0]：成功
	szret[1]：错误
*/
static const char* szret[] = { "I get a correct result\n", "Something wrong\n" };

/*
	从状态机，用于解析一行内容
	buffer		 ：待分析的行
	checked_index：指向当前正在分析的字节
	read_index   ：指向buffer中客户数据的尾部的下一字节
	
	0~checked_index 的字节已经分析完成
*/
LINE_STATUS parse_line( char* buffer, int& checked_index, int& read_index )
{
    char temp;
    for ( ; checked_index < read_index; ++checked_index )    
    {
        temp = buffer[ checked_index ];
        if ( temp == '\r' )                                  /*\r：回车符*/
        {
            if ( ( checked_index + 1 ) == read_index )
            {
                return LINE_OPEN;
            }
            else if ( buffer[ checked_index + 1 ] == '\n' ) /*\n:换行符  有回车不一定有新行*/
            {
                buffer[ checked_index++ ] = '\0';          
                buffer[ checked_index++ ] = '\0';           /*转义 两个'\0'表结束*/
                return LINE_OK;
            }
            return LINE_BAD;
        }
        else if( temp == '\n' )                           /*window 换行: \r\n  linux：\n*/
        {
            if( ( checked_index > 1 ) &&  buffer[ checked_index - 1 ] == '\r' )
            {
                buffer[ checked_index-1 ] = '\0';
                buffer[ checked_index++ ] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_OPEN;  
}
/*
	分析请求行
	szTemp    : 待分析的行
	checkstate：主状态机   
	http请求（例子） ：GET	http://www.baidu.com/index.html	HTTP/1.0\0\0
*/
HTTP_CODE parse_requestline( char* szTemp, CHECK_STATE& checkstate )
{
    char* szURL = strpbrk( szTemp, " \t" );     /*过滤掉GET :\thttp://.......*/
    if ( ! szURL )
    {
        return BAD_REQUEST;
    }
    *szURL++ = '\0';                           /* szTemp = GET\0*/

    char* szMethod = szTemp;
    if ( strcasecmp( szMethod, "GET" ) == 0 )
    {
        printf( "The request method is GET\n" );
    }
    else
    {
        return BAD_REQUEST;
    }

    szURL += strspn( szURL, " \t" );           /*szURL = http://www.baidu.com/index.html HTTP/1.0\0\0 */
    char* szVersion = strpbrk( szURL, " \t" ); /*szVersion = \tHTTP/1.0\0\0*/
    if ( ! szVersion )
    {
        return BAD_REQUEST;
    }
    *szVersion++ = '\0';				      /*szURL = http://www.baidu.com/index.html\0*/
    szVersion += strspn( szVersion, " \t" ); /*szVersion: HTTP/1.0\0\0*/
    if ( strcasecmp( szVersion, "HTTP/1.1" ) != 0 )
    {
        return BAD_REQUEST;
    }

    if ( strncasecmp( szURL, "http://", 7 ) == 0 )
    {
        szURL += 7;
        szURL = strchr( szURL, '/' );
    }

    if ( ! szURL || szURL[ 0 ] != '/' )
    {
        return BAD_REQUEST;
    }

    //URLDecode( szURL );
    printf( "The request URL is: %s\n", szURL );
    checkstate = CHECK_STATE_HEADER;   /*设置 进入分析头部模式*/
    return NO_REQUEST;
}

/*
	分析头部字段
	szTemp ：待分析头部
*/
HTTP_CODE parse_headers( char* szTemp )
{
    if ( szTemp[ 0 ] == '\0' )
    {
        return GET_REQUEST;
    }
    else if ( strncasecmp( szTemp, "Host:", 5 ) == 0 )
    {
        szTemp += 5;
        szTemp += strspn( szTemp, " \t" );
        printf( "the request host is: %s\n", szTemp );
    }
    else
    {
        printf( "I can not handle this header\n" );
    }

    return NO_REQUEST;
}
/*
	分析函数主入口
*/
HTTP_CODE parse_content( char* buffer, int& checked_index, CHECK_STATE& checkstate, int& read_index, int& start_line )
{
    LINE_STATUS linestatus = LINE_OK; /*记录当前行的状态*/
    HTTP_CODE retcode = NO_REQUEST;   /*记录http请求的处理结果*/
	
	/*主状态机  从buffer中取出所有完整的行*/
    while( ( linestatus = parse_line( buffer, checked_index, read_index ) ) == LINE_OK )
    {
        char* szTemp = buffer + start_line;    /*start_line 是 buffer的起始位置*/
        start_line = checked_index;            /*记录下一行的起始位置*/
        switch ( checkstate )
        {
            case CHECK_STATE_REQUESTLINE:      /*状态一：分析请求行*/
            {
                retcode = parse_requestline( szTemp, checkstate ); 
                if ( retcode == BAD_REQUEST )
                {
                    return BAD_REQUEST;
                }
                break;
            }
            case CHECK_STATE_HEADER:         /*状态二：分析头部字段*/
            {
                retcode = parse_headers( szTemp );
                if ( retcode == BAD_REQUEST )
                {
                    return BAD_REQUEST;
                }
                else if ( retcode == GET_REQUEST )
                {
                    return GET_REQUEST;
                }
                break;
            }
            default:
            {
                return INTERNAL_ERROR;
            }
        }
    }
	/* 如果没有读取到一个完整的行，则表示还需要继续读取客户数据才能进一步分析*/
    if( linestatus == LINE_OPEN )
    {
        return NO_REQUEST;
    }
    else
    {
        return BAD_REQUEST;
    }
}

int main( int argc, char* argv[] )
{
    if( argc <= 2 )
    {
        printf( "usage: %s ip_address port_number\n", basename( argv[0] ) );
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi( argv[2] );
    
    struct sockaddr_in address;
    bzero( &address, sizeof( address ) );
    address.sin_family = AF_INET;
    inet_pton( AF_INET, ip, &address.sin_addr );
    address.sin_port = htons( port );
    
    int listenfd = socket( PF_INET, SOCK_STREAM, 0 );
    assert( listenfd >= 0 );
    
    int ret = bind( listenfd, ( struct sockaddr* )&address, sizeof( address ) );
    assert( ret != -1 );
    
    ret = listen( listenfd, 5 );
    assert( ret != -1 );
    
    struct sockaddr_in client_address;
    socklen_t client_addrlength = sizeof( client_address );
    int fd = accept( listenfd, ( struct sockaddr* )&client_address, &client_addrlength );
    if( fd < 0 )
    {
        printf( "errno is: %d\n", errno );
    }
    else
    {
        char buffer[ BUFFER_SIZE ];          /*读取缓冲区*/
        memset( buffer, '\0', BUFFER_SIZE );
        int data_read = 0;
        int read_index = 0;                  /*当前已经读取了多少字节的客户数据*/
        int checked_index = 0;				 /* 当前已经分析了多少字节的客户数据*/
        int start_line = 0;					/*行在buffer中的起始位置*/
        CHECK_STATE checkstate = CHECK_STATE_REQUESTLINE;
        while( 1 )
        {
            data_read = recv( fd, buffer + read_index, BUFFER_SIZE - read_index, 0 );
            if ( data_read == -1 )
            {
                printf( "reading failed\n" );
                break;
            }
            else if ( data_read == 0 )
            {
                printf( "remote client has closed the connection\n" );
                break;
            }
    
            read_index += data_read;
            HTTP_CODE result = parse_content( buffer, checked_index, checkstate, read_index, start_line );
            if( result == NO_REQUEST )
            {
                continue;
            }
            else if( result == GET_REQUEST )
            {
                send( fd, szret[0], strlen( szret[0] ), 0 );
                break;
            }
            else
            {
                send( fd, szret[1], strlen( szret[1] ), 0 );
                break;
            }
        }
        close( fd );
    }
    
    close( listenfd );
    return 0;
}
