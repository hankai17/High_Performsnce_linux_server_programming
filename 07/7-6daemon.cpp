
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>

int deamonize(int nochdir, int noclose)
{
	pid_t pid = fork();
	if (pid < 0)
	{
		return -1;
	}
	else if (pid > 0) /*parent*/
	{
		exit(0);
	}
	
	/*child*/
	umask(0);
	/*create new session*/
	pid_t sid = setsid();
	if (sid < 0)
	{
		return -1;
	}		

	/*chdir*/
	if (!nochdir)
	{
		if ( (chdir("/")) < 0)
		{
			return -1;
		}
	}
	/*redirection*/
	if (!noclose)
	{
		close( STDIN_FILENO);
		close( STDOUT_FILENO);
		close( STDERR_FILENO);
		
		open("/dev/null", O_RDONLY);
		dup(0);
		dup(1);
	}
	
	return 0;
}

int main(void)
{
	deamonize(1,1);
	return 0;
}
