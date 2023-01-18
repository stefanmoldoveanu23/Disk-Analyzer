#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../dstructs/task.h"
#include "threads_manager.h"
#include "forks_manager.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define PORT 8080

volatile sig_atomic_t cnt_forks = 0;
volatile sig_atomic_t cnt_threads = 0;

volatile sig_atomic_t done = 0;
pid_t pid;

void child_done_handler(int signum) {
	wait(NULL);
	--cnt_forks;
}

void thread_start_handler(int signum) {
	++cnt_threads;
}

void thread_done_handler(int signum) {
	--cnt_threads;
}

void resman_handler(int signum) {
	++done;
	if (done >= 3) {
		signal(SIGTERM, SIG_IGN);
	}
}

void reqman_handler(int signum) {
	signal(SIGTERM, resman_handler);
	done = 1;
}

void forkman_handler2(int signum) {
	signal(SIGTERM, SIG_IGN);
	done = 1;
}

void forkman_handler1(int signum) {
	signal(SIGTERM, forkman_handler2);
	kill(pid, SIGTERM);
}

void forkchild_handler(int signum) {
	signal(SIGTERM, SIG_IGN);
	done = 1;
}

void do_threads_manager();

void *do_requests_manager(void *v);
void *do_responses_manager(void *v);

void *request_thread(void *v);

void do_forks_manager(struct forks_manager *man);

int main()
{
	daemon(1, 1);
	
	struct forks_manager fman;
	if (forks_startup(&fman)) {
		perror("Error starting fork manager.");
		return errno;
	}
	
	pid = fork();
	if (pid == -1) {
		perror("Error forking at startup.");
		return errno;
	}
	
	
	if (pid) {
		signal(SIGTERM, forkman_handler1);
		do_forks_manager(&fman);
	} else {
		signal(SIGTERM, reqman_handler);
		
		signal(SIGUSR1, thread_start_handler);
		signal(SIGUSR2, thread_done_handler);
		
		tree_clear(&(fman.tre));
		do_threads_manager();
	}
	
	return 0;
}

void do_threads_manager()
{
	
	struct threads_manager man;
	if (threads_startup(&man)) {
		perror("Error when starting up thread manager");
		exit(errno);
	}
	
	pthread_t requests_t, responses_t;
	
	while (pthread_create(&responses_t, NULL, do_responses_manager, &man));
	while (pthread_create(&requests_t, NULL, do_requests_manager, &man));
	
	
	pthread_join(requests_t, NULL);
	pthread_join(responses_t, NULL);
	
	threads_shutdown(&man);
}

void *do_requests_manager(void *v)
{
	struct threads_manager *man = (struct threads_manager *)v;
	
	int addlen = sizeof(man->requests_connection.address);
	
	while (!done) {
		pthread_mutex_lock(&(man->socket_mutex));
		man->requests_connection.client_fd = accept(man->requests_connection.server_fd, (struct sockaddr *)(&(man->requests_connection.address)), (socklen_t *)(&addlen));
		if (man->requests_connection.client_fd < 0) {
			pthread_mutex_unlock(&(man->socket_mutex));
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				continue;
			}
			
			perror("Error when accepting connection.");
			kill(getppid(), SIGTERM);
			break;
		}
		
		pthread_t thr;
		kill(getpid(), SIGUSR1);
		while (pthread_create(&thr, NULL, request_thread, (void*)(man)));
		
	}
	
	while (!done);
	
	while (cnt_threads);
	
	kill(getppid(), SIGTERM);

	pthread_exit(NULL);
}

void *request_thread(void *v)
{
	struct threads_manager *man = (struct threads_manager *)v;
	int client_fd = man->requests_connection.client_fd;
	pthread_mutex_unlock(&(man->socket_mutex));
	
	struct task tsk;
	if (readTask(client_fd, &tsk)) {
		perror(NULL);
		close(client_fd);
		kill(getpid(), SIGUSR2);
		return NULL;
	}
	
	struct analysis *anal = (struct analysis *)malloc(sizeof(struct analysis));
	
	if (!anal) {
		perror(NULL);
		free(tsk.path);
		close(client_fd);
		kill(getpid(), SIGUSR2);
		return NULL;
	}
	
	anal->total_time = 0;
	anal->path = strdup(tsk.path);
	if (!(anal->path)) {
		perror(NULL);
		free(tsk.path);
		free(anal);
		close(client_fd);
		kill(getpid(), SIGUSR2);
		return NULL;
	}
	anal->status = ANALYSIS_PENDING;
	anal->suspended = ANALYSIS_RESUMED;
	
	free(tsk.path);
	
	if (threads_add(man, anal)) {
		perror(NULL);
		free(anal->path);
		free(anal);
	}

	close(client_fd);
	pthread_detach(pthread_self());
	kill(getpid(), SIGUSR2);
	pthread_exit(NULL);
}

void *do_responses_manager(void *v)
{
	struct threads_manager *man = (struct threads_manager *)v;
	
	int addlen = sizeof(man->responses_connection.address);
	while (done != 3) {
		man->responses_connection.client_fd = accept(man->responses_connection.server_fd, (struct sockaddr *)(&(man->responses_connection.address)), (socklen_t *)(&addlen));
		if (man->responses_connection.client_fd < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				continue;
			}
			
			perror("Error when accepting connection.");
			kill(getppid(), SIGTERM);
			break;
		}
		
		char buffer[11];
		buffer[10] = '\0';
		
		int left = 10;
		while (left) {
			int cnt = read(man->responses_connection.client_fd, buffer + 10 - left, left);
			if (cnt < 0) {
				break;
			}
			
			left -= cnt;
		}
		
		if (!left) {
			int id = atoi(buffer);
			
			char result;
			int rd = 1;
			while (1) {
				rd = read(man->responses_connection.client_fd, &result, 1);
				if (rd) {
					break;
				}
			}
			
			if (rd == 1) {
				
				if (result == '0') {
					threads_remove(man, id);
				}
			}
		}
		
		close(man->responses_connection.client_fd);
	}
	
	while (done != 3);
	
	return NULL;
}

void do_forks_manager(struct forks_manager *man)
{
	int addlen = sizeof(man->connection.address);
	while (!done) {
		man->connection.client_fd = accept(man->connection.server_fd, (struct sockaddr *)(&(man->connection.address)), (socklen_t *)(&addlen));
		if (man->connection.client_fd < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				continue;
			}
			
			perror("Error when accepting connection.");
			kill(getpid(), SIGTERM);
			break;
		}

		int ret = forks_add(man, &done, forkchild_handler);
		if (ret < 0) {
			perror("Error adding new fork.");
			kill(getpid(), SIGTERM);
			break;
		} else if (!ret) {
			return;
		}
	}
	
	while (!done);
	
	kill(0, SIGTERM);
	
	while (cnt_forks);
	
	kill(pid, SIGTERM);
	
	forks_shutdown(man);
}
