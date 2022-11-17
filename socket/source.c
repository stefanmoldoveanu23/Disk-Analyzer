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
	
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		perror(NULL);
		return errno;
	}
	
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror(NULL);
		return errno;
	}
	
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	
	if (bind(server_fd, (struct sockaddr*)(&address), sizeof(address)) < 0) {
		perror(NULL);
		return errno;
	}
	
	if (listen(server_fd, 10000) < 0) {
		perror(NULL);
		return errno;
	}
	
	int client_fd = accept(server_fd, (struct sockaddr*)(&address), (socklen_t*)(&addrlen));
	if (client_fd < 0) {
		perror(NULL);
		return errno;
	}
	
	char buffer[1024] = { 0 };
	char *hello = "Hello from server!";
	
	int valread = read(client_fd, buffer, 1024);
	syslog(LOG_NOTICE, "%s\n", buffer);
	
	send(client_fd, hello, strlen(hello), 0);
	
	close(client_fd);
	shutdown(server_fd, SHUT_RDWR);
	closelog();
	
	return 0;
}
