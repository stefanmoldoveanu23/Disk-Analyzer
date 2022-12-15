#ifndef THREAD_MANAGER_H
#define THREAD_MANAGER_H

#include <pthread.h>

#define ID_MAX 100000

#define S_PENDING 0
#define S_INPROGRESS 1
#define S_COMPLETE 2

#define T_SUSPENDED 1
#define T_RESUMED 0

struct thread_manager{
	pthread_mutex_t analyses_mutex;
	struct treap *analyses;
	
	pthread_mutex_t available_mutex;
	struct treap *available_ids;
};

// tries to double the size of the analyses array
// if program has error, return 1; otherwise return 0
int reset_size(struct thread_manager *man, int size);

// handles manager startup
// if program has error, return 1; otherwise return 0
int handle_startup(struct thread_manager *man);
// if analysis is included in an already existing one, return -2
// if encounters error, return -1
// otherwise, returns id of new analysis
int add_task(struct thread_manager *man, const char *path);

// tries to remove analysis by id
// if no analysis with given id exists, return -1
// if other error occurs, return 1
// otherwise, return 0
int remove_task(struct thread_manager *man, const int id);

// tries to suspend analysis by id
// if no analysis with given id exists, return -2
// if analysis is suspended already, return -1
// otherwise, return 0
int suspend_task(struct thread_manager *man, const int id);

// tries to resume analysis by id
// if no analysis with given id exists, return -2
// if analysis is not suspended, return -1
// otherwise, return 0
int resume_task(struct thread_manager *man, const int id);

// returns status of analysis by id
// if no analysis with given id exists, return -1
// otherwise, return status
int get_status(const struct thread_manager man, const int id);

// returns result of analysis by id
// if no analysis with given id exists, return NULL
// if any other error occurs, return NULL(set errno as 0 at beginning)
// otherwise, return result as string
int get_result(const struct thread_manager man, const int id, char *result);

// return all analysis tasks, with id and path
// if error occurs, return NULL
// otherwise, return summary as string
int get_analyses(const struct thread_manager man, char *result);


void handle_shutdown(const struct thread_manager man);
#endif
