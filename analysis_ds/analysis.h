#ifndef ANALYSIS_H
#define ANALYSIS_H

#include <sys/types.h>

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

struct treap{
	int id;
	int heap_val;
	
	struct analysis *anal;
	
	struct treap *chld_left;
	struct treap *chld_right;
};

struct treap *create_node(const int id);


int insert_treap_id(struct treap **trp, const int id);


void insert_treap_node(struct treap **trp, struct treap *node);


int remove_treap(struct treap **trp, const int id);


struct treap *extract_treap(struct treap **trp, const int id);


void clear_treap(struct treap **trp);

#endif
