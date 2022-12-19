#ifndef THREAD_MANAGER_H
#define THREAD_MANAGER_H

#include <pthread.h>

#include "../dstructs/treap.h"
#include "../dstructs/create_socket.h"


struct thread_manager{
	int analysis_cnt;
	struct socket_connection connection;
	
	pthread_mutex_t analyses_mutex;
	struct treap *analyses;
	
	pthread_mutex_t available_mutex;
	struct treap *available_ids;
};


int handle_startup(struct thread_manager *man);


int add_task(struct thread_manager *man, const char *path);


int remove_task(struct thread_manager *man, const int id);


int suspend_task(struct thread_manager *man, const int id);


int resume_task(struct thread_manager *man, const int id);


int get_status(const struct thread_manager man, const int id);


int get_result(const struct thread_manager man, const int id, char *result);


int get_analyses(const struct thread_manager man, char *result);


void handle_shutdown(struct thread_manager *man);
#endif
