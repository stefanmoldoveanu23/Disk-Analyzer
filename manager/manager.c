#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../dstructs/task.h"
#include "requests_manager.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define PORT 8080

volatile sig_atomic_t done = 0;

int cnt_threads = 0;
pthread_mutex_t cnt_mutex;

void increase_cnt();
void decrease_cnt();

void do_requests_manager();
void *request_thread(void *v);

void do_forks_manager();

int main()
{
	daemon(1, 1);
	
	pid_t pid = fork();
	if (pid == -1) {
		perror("Error forking at startup.");
		return errno;
	}
	
	if (pid) {
		do_requests_manager();
	} else {
		do_forks_manager();
	}
	
	return 0;
}

void do_requests_manager()
{
	int reqs = 2;
	
	if (pthread_mutex_init(&cnt_mutex, NULL)) {
		perror("Error when starting thread counter mutex.");
		exit(errno);
	}
	
	struct requests_manager man;
	if (requests_startup(&man)) {
		perror("Error when starting up thread manager");
		pthread_mutex_destroy(&cnt_mutex);
		exit(errno);
	}
	
	int addlen = sizeof(man.connection.address);
	for (int i = 0; i < reqs; ++i) {
		pthread_mutex_lock(&(man.socket_mutex));
		man.connection.client_fd = accept(man.connection.server_fd, (struct sockaddr *)(&(man.connection.address)), (socklen_t *)(&addlen));
		if (man.connection.client_fd < 0) {
			perror("Error when accepting connection.");
			break;
		}
		
		pthread_t thr;
		while (pthread_create(&thr, NULL, request_thread, (void*)(&man)));
		
	}
	
	while (1) {
		pthread_mutex_lock(&cnt_mutex);
		if (!cnt_threads) {
			pthread_mutex_unlock(&cnt_mutex);
			break;
		}
		pthread_mutex_unlock(&cnt_mutex);
	}

	requests_shutdown(&man);
	pthread_mutex_destroy(&cnt_mutex);
	
	pthread_exit(NULL);
}

void *request_thread(void *v)
{
	increase_cnt();
	
	struct requests_manager *man = (struct requests_manager *)(v);
	int client_fd = man->connection.client_fd;
	pthread_mutex_unlock(&(man->socket_mutex));
	
	struct task tsk;
	if (readTask(client_fd, &tsk)) {
		perror(NULL);
		close(client_fd);
		decrease_cnt();
		return NULL;
	}
	
	struct analysis *anal = (struct analysis *)malloc(sizeof(struct analysis));
	
	if (!anal) {
		perror(NULL);
		free(tsk.path);
		close(client_fd);
		decrease_cnt();
		return NULL;
	}
	
	anal->total_time = 0;
	anal->path = strdup(tsk.path);
	if (!(anal->path)) {
		perror(NULL);
		free(tsk.path);
		free(anal);
		close(client_fd);
		decrease_cnt();
		return NULL;
	}
	anal->status = ANALYSIS_PENDING;
	anal->suspended = ANALYSIS_RESUMED;
	
	free(tsk.path);
	
	if (requests_add(man, anal)) {
		perror(NULL);
		free(anal->path);
		free(anal);
	}

	close(client_fd);
	
	decrease_cnt();
	pthread_detach(pthread_self());
	
	pthread_exit(NULL);
}

void increase_cnt()
{
	pthread_mutex_lock(&cnt_mutex);
	++cnt_threads;
	pthread_mutex_unlock(&cnt_mutex);
}
void decrease_cnt()
{
	pthread_mutex_lock(&cnt_mutex);
	--cnt_threads;
	pthread_mutex_unlock(&cnt_mutex);
}

void do_forks_manager()
{
	
}
