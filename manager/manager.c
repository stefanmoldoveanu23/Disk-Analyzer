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

pthread_mutex_t counter_mutex;
int cnt_threads = 0;

void incr_counter()
{
	pthread_mutex_lock(&counter_mutex);
	++cnt_threads;
	pthread_mutex_unlock(&counter_mutex);
}

void decr_counter()
{
	pthread_mutex_lock(&counter_mutex);
	--cnt_threads;
	pthread_mutex_unlock(&counter_mutex);
}


volatile sig_atomic_t cnt_forks = 0;
volatile sig_atomic_t cnt_need = 0;

volatile sig_atomic_t done = 0;
volatile sig_atomic_t interrupted = 0;
volatile sig_atomic_t suspended = 0;

pid_t pid;

void child_done_handler(int signum) {
	wait(NULL);
	--cnt_forks;
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
	switch (signum) {
		case SIGTERM: {
			signal(SIGTERM, SIG_IGN);
			done = 1;
			break;
		}
		case SIGINT: {
			signal(SIGINT, SIG_IGN);
			interrupted = 1;
			break;
		}
		case SIGTSTP: {
			suspended = 1;
			break;
		}
		case SIGCONT: {
			suspended = 0;
			break;
		}
	}
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
		fman.done = &done;
		fman.interrupted = &interrupted;
		fman.suspended = &suspended;
		fman.handler = forkchild_handler;
		
		signal(SIGTERM, forkman_handler1);
		signal(SIGCHLD, child_done_handler);
		
		do_forks_manager(&fman);
	} else {
		signal(SIGTERM, reqman_handler);
		
		tree_clear(&(fman.tre));
		do_threads_manager();
	}
	
	return 0;
}

void do_threads_manager()
{
	if (pthread_mutex_init(&counter_mutex, NULL)) {
		perror("Error when initializing thread counter mutex");
		exit(errno);
	}
	
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
	pthread_mutex_destroy(&counter_mutex);
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
		incr_counter();
		while (pthread_create(&thr, NULL, request_thread, (void*)(man)));
		
	}
	
	while (!done);
	
	pthread_mutex_lock(&counter_mutex);
	while (cnt_threads) {
		pthread_mutex_unlock(&counter_mutex);
	}
	pthread_mutex_unlock(&counter_mutex);
	
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
		decr_counter();
		return NULL;
	}
	
	switch (tsk.cnt) {
		case 1: {
			struct analysis *anal = (struct analysis *)malloc(sizeof(struct analysis));
	
			if (!anal) {
				perror(NULL);
				free(tsk.path);
				close(client_fd);
				decr_counter();
				return NULL;
			}
			
			anal->total_time = 0;
			anal->path = strdup(tsk.path);
			if (!(anal->path)) {
				perror(NULL);
				free(tsk.path);
				free(anal);
				close(client_fd);
				decr_counter();
				return NULL;
			}
			
			anal->priority = tsk.priority;
			anal->status = ANALYSIS_PENDING;
			anal->suspended = ANALYSIS_RESUMED;
			anal->cnt_dirs = anal->cnt_files = 0;
			
			free(tsk.path);
			
			if (threads_add(man, anal, client_fd)) {
				perror(NULL);
				free(anal->path);
				free(anal);
			}

			
			break;
		}
		case 2: {
			threads_suspend(man, tsk.id, client_fd);
			break;
		}
		case 3: {
			threads_resume(man, tsk.id, client_fd);
			break;
		}
		case 4: {
			threads_remove(man, tsk.id, client_fd);
			break;
		}
	}
	
	

	close(client_fd);
	decr_counter();
	pthread_detach(pthread_self());
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
		
		if (threads_read_results(man)) {
			perror("Error reading results");
		}
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

		int ret = forks_add(man);
		if (ret < 0) {
			perror("Error adding new fork.");
			kill(getpid(), SIGTERM);
			break;
		} else if (!ret) {
			return;
		}
		
		++cnt_forks;
	}
	
	while (!done);
	
	kill(0, SIGTERM);
	
	while (cnt_forks);
	
	kill(pid, SIGTERM);
	
	forks_shutdown(man);
}
