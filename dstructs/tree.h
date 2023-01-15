#ifndef TREE_H
#define TREE_H

#include "hash.h"

struct state{
	int size;
	int done;
};

struct tree{
	void *info;
	
	struct hash hsh;
};


int tree_init(struct tree **tre);


int tree_insert(struct tree *tre, char *path, void *info);


int tree_find_prefix(const struct tree *tre, char *path, void **info);


int tree_remove(struct tree *tre, const char *path);


void tree_clear(struct tree **tre);


void state_write(struct state *st, int fd);


void tree_save(struct tree **tre, int fd);

#endif
