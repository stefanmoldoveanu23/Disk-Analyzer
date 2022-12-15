#include "thread_manager.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define ANALYSES_PATH "../data/analyses"

// tries to double the size of the analyses array
// if program has error, return 1; otherwise return 0
int reset_size(struct thread_manager *man, int size)
{
	man->total_size = size;
	man->analyses = (struct analysis *)realloc(man->analyses, sizeof(struct analysis) * man->total_size);
	
	if (!(man->analyses)) {
		return 1;
	}
	
	return 0;
}


// handles manager startup
// if program has error, return 1; otherwise return 0
int handle_startup(struct thread_manager *man, int *server_fd)
{
	int fd = open(ANALYSES_PATH, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		return 1;
	}
	
	man->size = 1;
	man->analyses = (struct analyses *)malloc(man->analyses, sizeof(struct analysis));
	if (!(man->analyses)) {
		return 1;
	}
	
	struct analysis input;
	while (1) {
		// read analysis
	}
	
	return 0;
}
