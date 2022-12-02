#include "fcntl.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/wait.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "signal.h"
#include "time.h"

void parent_handle(int sgn)
{
	kill(0, sgn);
	wait(NULL);
	printf("Done.\n");
	exit(EXIT_SUCCESS);
}

void child_handle(int sgn)
{
	sleep(10);
	int fd = open("/home/characterme/Disk-Analyzer/setup/exit", O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR);
	write(fd, "Done.\n", 6);
	close(fd);
	exit(EXIT_SUCCESS);
}

int main()
{
	pid_t pid = fork();
	if (pid) {
		signal(SIGTERM, parent_handle);

		while (1);
	} else {
		signal(SIGTERM, child_handle);

		while (1);
	}
}
