#include <unistd.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 100

int main()
{
	char buf[BUFFER_SIZE];
	char *cwd;

	cwd = getcwd(buf, BUFFER_SIZE);
	if (!cwd) {
		perror("Getcwd returned error.\n");
		return errno;
	}

	printf("%s\n", cwd);

	return 0;
}
