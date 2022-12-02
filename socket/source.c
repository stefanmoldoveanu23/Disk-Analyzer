#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <pthread.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PORT 8080

int done = 0;
pthread_mutex_t mtx;

void *thread_func(void *v)
{
	int client_fd = *((int *)v);
		
	char buffer[1024] = { 0 };
	
	int valread = read(client_fd, buffer, 1024);
	
	if (buffer[0] == 'a') {
		close(client_fd);
		
		pthread_mutex_lock(&mtx);
		done = 1;
		pthread_mutex_unlock(&mtx);
		
		return NULL;
	}
	
	sleep(20);
	
	send(client_fd, buffer, strlen(buffer), 0);
	
	close(client_fd);
	return NULL;
}

int main()
{
	daemon(1, 1);
	
	if (pthread_mutex_init(&mtx, NULL)) {
		perror(NULL);
		return errno;
	}
	
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
	
	pthread_t tid[10000];
	int cnt = 0;
	
	
	fcntl(server_fd, F_SETFL, fcntl(server_fd, F_GETFL, 0) | O_NONBLOCK);
	while (1)
	{
		sleep(0.05);
		pthread_mutex_lock(&mtx);
		if (done) {
			pthread_mutex_unlock(&mtx);
			
			shutdown(server_fd, SHUT_RDWR);
			pthread_mutex_destroy(&mtx);
			
			for (int i = 0; i < cnt; ++i) {
				if (pthread_join(tid[i], NULL)) {
					perror(NULL);
					return errno;
				}
			}
			
			return 0;
		}
		pthread_mutex_unlock(&mtx);
		
		errno = 0;
		int client_fd = accept(server_fd, (struct sockaddr*)(&address), (socklen_t*)(&addrlen));
		if (client_fd == -1) {
			if (errno != EAGAIN && errno != EWOULDBLOCK) {
				perror(NULL);
				return errno;
			}
			continue;
		}
		
		int *p = (int *)malloc(sizeof(int));
		if (!p) {
			perror(NULL);
			return errno;
		}
		*p = client_fd;
		
		if (pthread_create(tid + (cnt++), NULL, thread_func, (void *)p)) {
			perror(NULL);
			return errno;
		}
		
	}
	
	shutdown(server_fd, SHUT_RDWR);
	
	pthread_mutex_destroy(&mtx);
	
	return 0;
}
