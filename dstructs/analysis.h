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

	// general info
	char *path;
	int status;
	int suspended;

	// id of process that does the analysis
	pid_t pid;
};


int analysis_read(int *id, struct analysis *anal, int fd);


int analysis_write(const int id, const struct analysis *anal, int fd);

#endif
