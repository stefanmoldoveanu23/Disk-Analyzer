#include "create_socket.h"

#include <sys/types.h>
#include <unistd.h>

int create_socket_acceptor(struct socket_connection *connection, const int port)
{
	connection->server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (connection->server_fd < 0) {
		return 1;
	}
	
	int opt = 1;
	if (setsockopt(connection->server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		close(connection->server_fd);
		return 1;
	}
	
	connection->address.sin_family = AF_INET;
	connection->address.sin_addr.s_addr = INADDR_ANY;
	connection->address.sin_port = htons(port);
	
	if (bind(connection->server_fd, (struct sockaddr*)(&(connection->address)), sizeof(connection->address)) < 0) {
		close(connection->server_fd);
		return 1;
	}
	
	if (listen(connection->server_fd, 10000) < 0) {
		close(connection->server_fd);
		return 1;
	}
	
	return 0;
}

int create_socket_connector(struct socket_connection *connection, const int port)
{
	connection->client_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (connection->client_fd < 0) {
		return 1;
	}
	
	connection->address.sin_family = AF_INET;
	connection->address.sin_port = htons(port);
	
	if (inet_pton(AF_INET, "127.0.0.1", &(connection->address.sin_addr)) <= 0) {
		close(connection->client_fd);
		return 1;
	}
	
	return 0;
}
