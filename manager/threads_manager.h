#ifndef REQUESTS_MANAGER_H
#define REQUESTS_MANAGER_H

#include <pthread.h>

#include "../dstructs/treap.h"
#include "../dstructs/tree.h"
#include "../dstructs/create_socket.h"


struct threads_manager{
	pthread_mutex_t socket_mutex;
	struct socket_connection requests_connection;
	
	struct socket_connection responses_connection;
	
	pthread_mutex_t analyses_mutex;
	int analysis_cnt;
	int need;
	struct treap *analyses;
	
	pthread_mutex_t paths_mutex;
	struct tree *paths;
	
	pthread_mutex_t available_mutex;
	struct treap *available_ids;
};


int threads_startup(struct threads_manager *man);


int threads_read_results(struct threads_manager *man);


int threads_add(struct threads_manager *man, struct analysis *anal, int fd);


void threads_remove(struct threads_manager *man, const int id, int fd);


void threads_suspend(struct threads_manager *man, const int id, int fd);


void threads_resume(struct threads_manager *man, const int id, int fd);


void threads_status(struct threads_manager man, const int id, int fd);


int threads_result(struct threads_manager man, const int id, char *result, int fd);


int threads_get_all(struct threads_manager man, char *result, int fd);


void threads_shutdown(struct threads_manager *man);
#endif
