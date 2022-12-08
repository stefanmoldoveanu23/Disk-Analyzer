#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include "options_handler.h"
#include "errno.h"

int main(int argc, char *argv[])
{
	struct task *tsk;
	tsk = get_task(argc, argv);
	
	if (!tsk) {
		if (errno) {
			perror(NULL);
		}
		
		return 1;
	}
	
	char *string = taskToString(tsk);
	
	printf("%s\n", string);
	free(string);
	
	
	
	return 0;
}
