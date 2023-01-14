#include "tree.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


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
		printf("Have left: %s.\n", parse);
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


