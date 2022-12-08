#ifndef CREATE_SOCKET_H
#define CREATE_SOCKET_H

struct da_socket{
	int server_fd;
	int client_fd;
};

struct da_socket *create_socket_accept(int port);

struct da_socket *create_socket_connect(int port);

#endif
