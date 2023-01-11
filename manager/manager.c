#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../dstructs/task.h"
#include "requests_manager.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080

int main(int argc, char *argv[])
{
	daemon(1, 1);
	
	int reqs = atoi(argv[1]);
	
	struct requests_manager man;
	if (requests_startup(&man)) {
		perror("Error when starting up thread manager");
		return 1;
	}
	
	for (int i = 0; i < reqs; ++i) {
		int addlen = sizeof(man.connection.address);
		man.connection.client_fd = accept(man.connection.server_fd, (struct sockaddr *)(&(man.connection.address)), (socklen_t *)(&addlen));
		if (man.connection.client_fd < 0) {
			perror("Error when accepting connection");
			requests_shutdown(&man);
			return 1;
		}
		
		struct task tsk;
		if (readTask(man.connection.client_fd, &tsk)) {
			perror(NULL);
			close(man.connection.client_fd);
			requests_shutdown(&man);
			return 1;
		}
		
		struct analysis *anal = (struct analysis *)malloc(sizeof(struct analysis));
		
		if (!anal) {
			perror(NULL);
			free(tsk.path);
			close(man.connection.client_fd);
			continue;
		}
		
		anal->total_time = 0;
		anal->path = strdup(tsk.path);
		if (!(anal->path)) {
			perror(NULL);
			free(tsk.path);
			free(anal);
			close(man.connection.client_fd);
			continue;
		}
		anal->status = ANALYSIS_PENDING;
		anal->suspended = ANALYSIS_RESUMED;
		
		free(tsk.path);
		
		if (requests_add(&man, anal)) {
			perror(NULL);
			free(anal->path);
			free(anal);
		}

		close(man.connection.client_fd);
	}

	requests_shutdown(&man);
	
	return 0;
}
