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

void daemonize()
{
	pid_t pid = fork();
	if (pid < 0) {
		perror(NULL);
		exit(EXIT_FAILURE);
	}
	
	if (pid) {
		exit(EXIT_SUCCESS);
	}
	
	setsid();
	
	pid = fork();
	if (pid < 0) {
		perror(NULL);
		exit(EXIT_FAILURE);
	}
	
	if (pid) {
		exit(EXIT_SUCCESS);
	}
	
	umask(0);
	chdir("/");
	
	char id[15];
	sprintf(id, "%d", getpid());
	openlog(id, LOG_PID, LOG_DAEMON);
}

int main()
{
	daemonize();
	
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
	
	char buffer[1024] = { 0 };
	char *hello = "Hello from client!";
	
	if (send(client_fd, hello, strlen(hello), 0) == -1) {
		perror(NULL);
		return errno;
	}
	
	int valread = read(client_fd, buffer, 1024);
	syslog(LOG_NOTICE, "%s\n", buffer);
	
	close(server_fd);
	shutdown(client_fd, SHUT_RDWR);
	closelog();
	
	return 0;
}
