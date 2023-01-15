#include "tree.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


int tree_init(struct tree **tre)
{
	*tre = (struct tree *)malloc(sizeof(struct tree));
	if (!(*tre)) {
		perror("Error initializing tree");
		return 1;
	}
	
	(*tre)->info = NULL;
	(*tre)->hsh.size = 0;
	memset((*tre)->hsh.children, 0, HASH_MOD * sizeof(struct list *));
	
	return 0;
}


int tree_insert(struct tree *tre, char *path, void *info)
{
	char *parse = path;

	while (*(parse += strspn(parse, "/")) != '\0') {
		size_t len = strcspn(parse, "/");

		struct tree *nxt = hash_find(tre->hsh, parse);
		if (!nxt) {
			char *to_insert = strndup(parse, len);
			if (!to_insert) {
				perror("Error duplicating path to insert to tree");
				return 1;
			}

			tre = hash_insert(&(tre->hsh), to_insert);
			if (!tre) {
				perror("Error inserting element in hash");
				free(to_insert);
				return 1;
			}
			
		} else {
			tre = nxt;
		}

		parse += len;
	}
	
	if (tre->info) {
		free(tre->info);
	}
	tre->info = info;
	return 0;
}


int tree_find_prefix(const struct tree *tre, char *path, void **info)
{
	if (tre->info) {
		if (info) {
			*info = tre->info;
		}
		return 1;
	}

	char *parse = path;
	
	while (*(parse += strspn(parse, "/")) != '\0') {
		
		struct tree *nxt = hash_find(tre->hsh, parse);
		if (!nxt) {
			if (info) {
				*info = NULL;
			}
			return 0;
		}

		tre = nxt;
		if (tre->info) {
			if (info) {
				*info = tre->info;
			}
			
			return 1;
		}
		
		parse += strcspn(parse, "/");
	}
	
	return 0;
}


int tree_remove(struct tree *tre, const char *path)
{
	path += strspn(path, "/");
	if (*path == '\0') {
		if (tre->info) {
			free(tre->info);
			tre->info = NULL;
			return 0;
		}
		
		return 1;
	}
	
	struct tree *nxt = hash_find(tre->hsh, path);
	if (!nxt) {
		return 1;
	}
	
	path += strcspn(path, "/");
	if (tree_remove(nxt, path)) {
		return 1;
	}
	
	if (!(nxt->info) && !(nxt->hsh.size)) {
		hash_remove(&(tre->hsh), path);
	}
	
	return 0;
}


void tree_clear(struct tree **tre)
{
	if ((*tre)->info) {
		free((*tre)->info);
		(*tre)->info = NULL;
	}
	
	hash_clear(&((*tre)->hsh));
	free(*tre);
}


void tree_save(struct tree **tre, int fd) {
	state_write((struct state *)((*tre)->info), fd);
	
	if ((*tre)->info) {
		free((*tre)->info);
		(*tre)->info = NULL;
	}
	
	hash_save(&((*tre)->hsh), fd);
	free(*tre);
}


void state_write(struct state *st, int fd)
{
	if (!st) {
		write(fd, "0", 1);
	} else {
		write(fd, "1", 1);
		
		if (st->done) {
			write(fd, "1", 1);
		} else {
			write(fd, "0", 1);
		}
		
		char vessel[11];
		vessel[10] = '\0';
		
		snprintf(vessel, 11, "%010d", st->size);
		write(fd, vessel, 10);
	}
}
