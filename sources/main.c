#include "hello.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

int main()
{
	pid_t pid = fork();
	if (pid < 0) {
		perror(NULL);
		return errno;
	} else if (pid) {
		return 0;
	}
	
	setsid();
	
	pid = fork();
	if (pid < 0) {
		perror(NULL);
		return errno;
	} else if (pid) {
		return 0;
	}
	
	umask(0);
	chdir("/");
	
	hello_1();
	hello_2();

	return 0;
}
