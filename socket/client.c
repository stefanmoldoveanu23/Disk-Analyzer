#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 8080

int main(int argc, char *argv[])
{
	if (argc != 2) {
		perror("Wrong amount of arguments.\n");
		return 1;
	}
	
	if (daemon(1, 1)) {
		perror(NULL);
		return errno;
	}
	
	int client_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (client_fd < 0) {
		perror("Socket creation failed.\n");
		return errno;
	}
	
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	
	address.sin_family = AF_INET;
	address.sin_port = htons(PORT);
	
	if (inet_pton(AF_INET, "127.0.0.1", &(address.sin_addr)) <= 0) {
		perror("Invalid address/Address not supported.\n");
		return errno;
	}
	
	int server_fd = connect(client_fd, (struct sockaddr*)(&address), sizeof(address));
	if (server_fd < 0) {
		perror("Connection failed.\n");
		return errno;
	}
	
	
	if (send(client_fd, argv[1], strlen(argv[1]), 0) == -1) {
		perror(NULL);
		return errno;
	}
	
	if (argv[1][0] != 'a') {
		char buffer[1024] = { 0 };
	
		int valread = read(client_fd, buffer, 1024);
		printf("%s\n", buffer);
	}
	
	close(server_fd);
	shutdown(client_fd, SHUT_RDWR);
	
	return 0;
}
