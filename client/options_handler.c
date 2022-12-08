#include "options_handler.h"
#include "errno.h"
#include "stdlib.h"
#include "stdio.h"

const char *options = "a:p:S:R:r:i:l";
const struct option long_options[] = {
		{"add", required_argument, 0, 'a'},
		{"priority", required_argument, 0, 'p'},
		{"suspend", required_argument, 0, 'S'},
		{"resume", required_argument, 0, 'R'},
		{"remove", required_argument, 0, 'r'},
		{"info", required_argument, 0, 'i'},
		{"list", no_argument, 0, 'l'},
		{"print", required_argument, 0, 'p'},
		{0, 0, 0, 0}
	};

struct task *get_task(int argc, char **argv)
{
	struct task *tsk = (struct task*)malloc(sizeof(struct task));
	if (!tsk){
		return NULL;
	}
	
	errno = 0;
	int opt_1 = getopt_long(argc, argv, options, long_options, NULL);
	if (errno) {
		free(tsk);
		return NULL;
	}
	
	switch (opt_1) {
		case -1: {
			perror("Need option.\n");
			free(tsk);
			return NULL;
		}
		case 'a': {
			tsk->cnt = 1;
			tsk->path = optarg;
			
			break;
		}
		case 'l': {
			tsk->cnt = 6;
			break;
		}
		case '?': {
			perror("Nonexistent option.\n");
			free(tsk);
			return NULL;
		}
		case 'S': {
			tsk->cnt = 2;
			tsk->id = atoi(optarg);
			break;
		}
		case 'R': {
			tsk->cnt = 3;
			tsk->id = atoi(optarg);
			break;
		}
		case 'r': {
			tsk->cnt = 4;
			tsk->id = atoi(optarg);
			break;
		}
		case 'i': {
			tsk->cnt = 5;
			tsk->id = atoi(optarg);
			break;
		}
		case 'p': {
			tsk->cnt = 7;
			tsk->id = atoi(optarg);
			break;
		}
	}
	
	int opt_2 = getopt_long(argc, argv, options, long_options, NULL);
	if (errno) {
		if (tsk->cnt == 1) {
			free(tsk->path);
		}
		free(tsk);
		return NULL;
	}
	
	if (tsk->cnt != 1) {
		if (opt_2 != -1) {
			perror("Too many arguments.\n");
			free(tsk);
			return NULL;
		} else {
			if (optind < argc) {
				perror("Too much input.\n");
				free(tsk);
				return NULL;
			}
			
			return tsk;
		}
	} else {
		if (opt_2 == -1) {
			tsk->priority = 2;
		} else if (opt_2 != 'p') {
			perror("Only possible argument after -a is -p.\n");
			free(tsk);
			return NULL;
		} else {
			tsk->priority = atoi(optarg);
		}
	}
	
	int opt_3 = getopt_long(argc, argv, options, long_options, NULL);
	if (errno) {
		free(tsk);
		return NULL;
	}
	
	if (opt_3 != -1) {
		perror("Too many options!\n");
		free(tsk);
		return NULL;
	}
	
	if (optind < argc) {
		printf("Too much input.\n");
		free(tsk);
		return NULL;
	}
	
	return tsk;
}
