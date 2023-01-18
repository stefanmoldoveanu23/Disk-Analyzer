#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "options_handler.h"
#include "../dstructs/create_socket.h"

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
	
	free(tsk.path);
	
	struct socket_connection connection;
	if (create_socket_connector(&connection, PORT)) {
		perror(NULL);
		free(request);
		
		return 1;
	}
	
	connection.server_fd = connect(connection.client_fd, (struct sockaddr*)(&connection.address), sizeof(connection.address));
	if (connection.server_fd < 0) {
		perror(NULL);
		free(request);
		shutdown(connection.client_fd, SHUT_RDWR);
		return 1;
	}
	
	int pos = 0, total = strlen(request);
	while (pos != total) {
		int sent = send(connection.client_fd, request + pos, total - pos, 0);
		
		if (sent < 0) {
			perror(NULL);
			free(request);
			close(connection.server_fd);
			shutdown(connection.client_fd, SHUT_RDWR);
			
			return errno;
		}
		
		pos += sent;
	}
	
	free(request);
	
	close(connection.server_fd);
	shutdown(connection.client_fd, SHUT_RDWR);
	
	return 0;
}
