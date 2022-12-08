#include "create_socket.h"

#include <sys/types.h>

struct socket_connection *create_socket_acceptor(int port)
{
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		return -1;
	}
	
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		return -1;
	}
	
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	
	if (bind(server_fd, (struct sockaddr*)(&address), sizeof(address)) < 0) {
		return -1;
	}
	
	if (listen(server_fd, 10000) < 0) {
		return -1;
	}
	
	struct socket_connection *connection = (struct socket_connection *)malloc(sizeof(struct socket_connection));
	if (!connection) {
		return -1;
	}
	
	connection->server_fd = server_fd;
	connection->address = address;
	
	return connection;
}

int create_socket_connector(int port)
{
	int client_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (client_fd < 0) {
		return -1;
	}
	
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	
	address.sin_family = AF_INET;
	address.sin_port = htons(PORT);
	
	if (inet_pton(AF_INET, "127.0.0.1", &(address.sin_addr)) <= 0) {
		return -1;
	}
	
	struct socket_connection *connection = (struct socket_connection *)malloc(sizeof(struct socket_connection));
	if (!connection) {
		return -1;
	}
	
	connection->client_fd = client_fd;
	connection->address = address;
	
	return connection;
}
