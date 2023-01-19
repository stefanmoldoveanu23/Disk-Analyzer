#ifndef ANALYSIS_H
#define ANALYSIS_H

#include <sys/types.h>

#define ANALYSIS_PENDING 0
#define ANALYSIS_INPROGRESS 1
#define ANALYSIS_COMPLETE 2

#define ANALYSIS_SUSPENDED 1
#define ANALYSIS_RESUMED 0

struct analysis{
	// total time spend analysing, and last analysis start time
	int total_time;
	int last_start;
	
	// progess data
	int cnt_files;
	int cnt_dirs;

	// general info
	char *path;
	int priority;
	int status;
	int suspended;

	// id of process that does the analysis
	pid_t pid;
};


int analysis_read(int *id, struct analysis *anal, int fd);


int analysis_write(const int id, const struct analysis *anal, int fd);


// messages to send to user
void analysis_custom_message(int fd, char *msg);


void analysis_created(int fd, int id, struct analysis *anal);

void analysis_path_already_exists(int fd, char *path, int id);

void analysis_id_no_exists(int fd, int id);

void analysis_suspended(int fd, int id, struct analysis *anal);

void analysis_resumed(int fd, int id, struct analysis *anal);

void analysis_removed(int fd, int id, struct analysis *anal);

void analysis_status(int fd, int id, struct analysis *anal);

void analysis_list(int fd, int id, struct analysis *anal);

void analysis_report(int in_fd, int id);

#endif
