#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

#include <stdio.h>
#include <string.h>

#define PORT 8080

int main()
{
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		perror("Socket creation failed.\n");
		return errno;
	}
	
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror("Setsockopt failed.\n");
		return errno;
	}
	
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	
	if (bind(server_fd, (struct sockaddr*)(&address), sizeof(address)) < 0) {
		perror("Bind failed.\n");
		return errno;
	}
	
	if (listen(server_fd, 10000) < 0) {
		perror("Listen failed.\n");
		return errno;
	}
	
	int client_fd = accept(server_fd, (struct sockaddr*)(&address), (socklen_t*)(&addrlen));
	if (client_fd < 0) {
		perror("Accept failed.\n");
		return errno;
	}
	
	char buffer[1024] = { 0 };
	char *hello = "Hello from server!";
	
	int valread = read(client_fd, buffer, 1024);
	printf("%s\n", buffer);
	
	send(client_fd, hello, strlen(hello), 0);
	
	close(client_fd);
	shutdown(server_fd, SHUT_RDWR);
	
	return 0;
}
