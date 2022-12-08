#ifndef CREATE_SOCKET_H
#define CREATE_SOCKET_H

#include "sys/socket.h"

struct socket_connection{
	int client_fd;
	int server_fd;
	struct sockaddr_in address;
};

/// create a socket and prepare it to accept connections on given port
struct socket_connection *create_socket_acceptor(int port);

/// create a socket and prepare it to connect to listening sockets on given port
struct socket_connection *create_socket_connector(int port);

#endif
