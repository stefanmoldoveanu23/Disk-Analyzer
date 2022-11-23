#ifndef MANAGER_H
#define MANAGER_H

#define S_PENDING 0
#define S_INPROGRESS 1
#define S_COMPLETE 2

#define T_SUSPENDED 1
#define T_RESUMED 0

struct analysis{
	int status;
	int suspended;
	
	char *path;
};

struct manager{
	int ceiling;
	struct analysis **analyses;
};

// initiates manager with ceiling
// if program has error, return 1; otherwise return 0
int init(struct manager *man, int ceiling);

// if analysis is included in an already existing one, return -2
// if encounters error, return -1
// otherwise, returns id of new analysis
int add_task(struct manager *man, char *path);

// tries to remove analysis by id
// if no analysis with given id exists, return -1
// if other error occurs, return 1
// otherwise, return 0
int remove_task(struct manager *man, int id);

// tries to suspend analysis by id
// if no analysis with given id exists, return -2
// if analysis is suspended already, return -1
// otherwise, return 0
int suspend_task(struct manager *man, int id);

// tries to resume analysis by id
// if no analysis with given id exists, return -2
// if analysis is not suspended, return -1
// otherwise, return 0
int resume_task(struct manager *man, int id);

// returns status of analysis by id
// if no analysis with given id exists, return -1
// otherwise, return status
int get_status(struct manager *man, int id);

// returns result of analysis by id
// if no analysis with given id exists, return NULL
// if any other error occurs, return NULL(set errno as 0 at beginning)
// otherwise, return result as string
char *get_result(struct manager *man, int id);

// return all analysis tasks, with id and path
// if error occurs, return NULL
// otherwise, return summary as string
char *get_manager(struct manager *man);

#endif
