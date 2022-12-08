#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "options_handler.h"
//#include "../socket/create_socket.h"

#define PORT 8080

int main(int argc, char *argv[])
{
	daemon(1, 1);
	
	struct task tsk;
	
	if (get_task(argc, argv, &tsk)) {
		if (errno) {
			perror(NULL);
		}
		
		return 1;
	}
	
	char *request;
	
	if (taskToString(tsk, &request)) {
		return 1;
	}
	
	printf("%s\n", request);
	
	free(request);
	
	/*struct socket_connection *connection = create_socket_connector(PORT);
	if (!connection) {
		perror(NULL);
		free(request);
		free(tsk);
		return errno;
	}
	
	int pos = 0, total = strlen(request);
	while (pos != total) {
		int sent = send(connection->client_fd, request + pos, total - pos, 0);
		
		if (sent < 0) {
			perror(NULL);
			free(request);
			free(tsk);
			close(connection->server_fd);
			shutdown(connection->client_fd, SHUT_RDWR);
			free(connection);
			
			return errno;
		}
		
		pos += sent;
	}
	
	free(string);
	free(tsk);
	
	close(connection->server_fd);
	shutdown(connection->client_fd, SHUT_RDWR);
	free(connection);*/
	
	return 0;
}
