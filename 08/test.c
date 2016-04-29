
#include <stdio.h>
#include <string.h>


int main(void)
{
	char http[] = "GET	http://www.baidu.com/index.html HTTP/1.0\0\0";
	
	char *szURL = strpbrk(http, "\t");
	
	printf("szURL: %s\n", szURL);
	
	*(szURL++) = '\0';

	
	printf("szURL: %s\n", szURL);
	printf("http: %s\n", http);
	
	
	return 0;
}