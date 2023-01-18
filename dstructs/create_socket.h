#ifndef CREATE_SOCKET_H
#define CREATE_SOCKET_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

struct socket_connection{
	int client_fd;
	int server_fd;
	struct sockaddr_in address;
};

/// create a socket and prepare it to accept connections on given port
int create_socket_acceptor(struct socket_connection *connection, const int port);

/// create a socket and prepare it to connect to listening sockets on given port
int create_socket_connector(struct socket_connection *connection, const int port);


int create_socket_send_message(char *buffer, int fd);

#endif
