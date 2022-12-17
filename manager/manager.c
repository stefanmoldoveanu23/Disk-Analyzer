#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//#include "../task/task.h"
//#include "../socket/create_socket.h"

//#include "thread_manager.h"

#include "../analysis_ds/analysis.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080

int main()
{
	srand(time(NULL));
	struct treap *trp = NULL;
	
	for (int i = 0; i < 100000; ++i) {
		if (insert_treap_id(&trp, i)) {
			perror("1");
			return 1;
		}
	}
	
	clear_treap(&trp);
	
	/*
	daemon(1, 1);
	
	struct socket_connection connection;
	if (create_socket_acceptor(&connection, PORT)) {
		perror(NULL);
		return 1;
	}
	
	int addlen = sizeof(connection.address);
	connection.client_fd = accept(connection.server_fd, (struct sockaddr *)(&connection.address), (socklen_t *)(&addlen));
	if (connection.client_fd < 0) {
		perror(NULL);
		shutdown(connection.server_fd, SHUT_RDWR);
		return 1;
	}
	
	struct task tsk;
	if (readTask(connection.client_fd, &tsk)) {
		perror(NULL);
		close(connection.client_fd);
		shutdown(connection.server_fd, SHUT_RDWR);
		return 1;
	}
	
	printf("Task: %d\n", tsk.cnt);
	switch (tsk.cnt) {
		case 1: {
			printf("Path: %s\n", tsk.path);
			printf("Priority: %d\n", tsk.priority);
			break;
		}
		case 6: {
			break;
		}
		default: {
			printf("Id: %d\n", tsk.id);
		}
	}
	
	if (tsk.cnt == 1) {
		free(tsk.path);
	}
	
	close(connection.client_fd);
	shutdown(connection.server_fd, SHUT_RDWR);
	*/
	
	
	return 0;
}
