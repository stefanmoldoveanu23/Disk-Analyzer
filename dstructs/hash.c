#include "hash.h"
#include "tree.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>


int get_key(const char *path)
{
	int length = strlen(path);
	int key = 0;
	
	for (int i = 0; i < length; ++i) {
		if (path[i] == '/') {
			break;
		}
		key = ((key + (int)(path[i])) * HASH_MULT + HASH_ADD) % HASH_MOD;
	}
	
	return key;
}


struct list *list_new_node(char *path)
{
	struct list *lst = (struct list *)malloc(sizeof(struct list));
	if (!lst) {
		return NULL;
	}
	
	lst->node = (struct tree *)malloc(sizeof(struct tree));
	if (!(lst->node)) {
		return NULL;
	}
	
	lst->node->info = NULL;
	lst->node->hsh.size = 0;
	memset(lst->node->hsh.children, 0, HASH_MOD * sizeof(struct list *));
	
	lst->next = NULL;
	lst->path = path;
	return lst;
}


struct tree *list_push(struct list **lst, char *path)
{
	if (!(*lst)) {
		*lst = list_new_node(path);
		if (!(*lst)) {
			return NULL;
		}
		return (*lst)->node;
	}
	
	return list_push(&((*lst)->next), path);
}


struct tree *list_find(const struct list *lst, const char *path)
{
	if (!lst) {
		return NULL;
	}
	
	size_t sz = strlen(lst->path);
	size_t len = strcspn(path, "/");
	if (len > sz) {
		sz = len;
	}

	if (!strncmp(lst->path, path, sz)) {
		return lst->node;
	}
	
	return list_find(lst->next, path);
}


int list_remove(struct list **lst, const char *path)
{
	if (!(*lst)) {
		return 1;
	}
	
	size_t sz = strlen((*lst)->path);
	size_t len = strcspn(path, "/");
	if (len > sz) {
		sz = len;
	}
	
	if (!strncmp((*lst)->path, path, sz)) {
		struct list *to_remove = *lst;
		*lst = (*lst)->next;
		
		free(to_remove->node);
		free(to_remove->path);
		free(to_remove);
		return 0;
	}
	
	return list_remove(&((*lst)->next), path);
}


void list_clear(struct list **lst)
{
	while (*lst) {
		struct list *to_clear = *lst;
		*lst = (*lst)->next;
		
		tree_clear(&(to_clear->node));
		free(to_clear->path);
		free(to_clear);
	}
}


void list_save(struct list **lst, int fd)
{
	while (*lst) {
		struct list *to_clear = *lst;
		*lst = (*lst)->next;
		
		write(fd, "+", 1);
		char vessel[11 + strlen(to_clear->path)];
		snprintf(vessel, 11 + strlen(to_clear->path), "%010d%s", (int)strlen(to_clear->path), to_clear->path);
		write(fd, vessel, 10 + strlen(to_clear->path));
		
		tree_save(&(to_clear->node), fd);
		free(to_clear->path);
		free(to_clear);
	}
}


struct tree *hash_insert(struct hash *hsh, char *path)
{
	int key = get_key(path);
	
	struct tree *tre = list_find(hsh->children[key], path);
	if (!tre) {
		tre = list_push(&(hsh->children[key]), path);

		if (tre) {
			++(hsh->size);
		}
	}
	
	return tre;
}


int hash_build(struct hash *hsh, int fd)
{
	char movement[2];
	char buffer[11];
	buffer[10] = '\0';
	
	while (1) {
		if (read(fd, movement, 1) < 1) {
			perror("Error reading");
			return 1;
		}
		
		if (movement[0] == '+') {
			if (read(fd, buffer, 10) < 10) {
				perror("Error reading");
				return 1;
			}
			int sz = atoi(buffer);
			
			char *name = (char *)malloc(sz + 1);
			if (!name) {
				perror("Error allocating memory to name of file");
				return 1;
			}
			name[sz] = '\0';
			
			if (read(fd, name, sz) != sz) {
				perror("Error reading");
				free(name);
				return 1;
			}
			
			struct tree *nxt = hash_insert(hsh, name);
			if (!nxt) {
				perror("Error inserting into hash");
				free(name);
				return 1;
			}
			
			if (tree_build(nxt, fd)) {
				return 1;
			}
		} else if (movement[0] == '-') {
			return 0;
		} else {
			printf("%c\n", movement[0]);
			perror("Incorrect sign");
			return 1;
		}
	}
}


struct tree *hash_find(const struct hash hsh, const char *path)
{
	int key = get_key(path);
	
	return list_find(hsh.children[key], path);
}


int hash_remove(struct hash *hsh, const char *path)
{
	int key = get_key(path);
	
	if (list_remove(&(hsh->children[key]), path)) {
		return 1;
	} else {
		--hsh->size;
		return 0;
	}
}


void hash_clear(struct hash *hsh) {
	for (int i = 0; i < HASH_MOD; ++i) {
		list_clear(&(hsh->children[i]));
	}
	
	hsh->size = 0;
}


void hash_save(struct hash *hsh, int fd)
{
	for (int i = 0; i < HASH_MOD; ++i) {
		list_save(&(hsh->children[i]), fd);
	}
	
	hsh->size = 0;
	write(fd, "-", 1);
}
