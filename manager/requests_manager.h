#ifndef REQUESTS_MANAGER_H
#define REQUESTS_MANAGER_H

#include <pthread.h>

#include "../dstructs/treap.h"
#include "../dstructs/tree.h"
#include "../dstructs/create_socket.h"


struct requests_manager{
	pthread_mutex_t socket_mutex;
	struct socket_connection connection;
	
	pthread_mutex_t analyses_mutex;
	int analysis_cnt;
	struct treap *analyses;
	
	pthread_mutex_t paths_mutex;
	struct tree *paths;
	
	pthread_mutex_t available_mutex;
	struct treap *available_ids;
};


int requests_startup(struct requests_manager *man);


int requests_add(struct requests_manager *man, struct analysis *anal);


int requests_remove(struct requests_manager *man, const int id);


int requests_suspend(struct requests_manager *man, const int id);


int requests_resume(struct requests_manager *man, const int id);


int requests_status(const struct requests_manager man, const int id);


int requests_result(const struct requests_manager man, const int id, char *result);


int requests_get_all(const struct requests_manager man, char *result);


void requests_shutdown(struct requests_manager *man);
#endif
