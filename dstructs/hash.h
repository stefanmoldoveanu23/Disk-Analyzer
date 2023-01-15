#ifndef HASH_H
#define HASH_H

#define HASH_MULT 437
#define HASH_ADD 47
#define HASH_MOD 1000

struct list{
	char *path;

	struct tree *node;
	struct list *next;
};

struct hash{
	int size;

	struct list *children[HASH_MOD];
};


int get_key(const char *path);


struct list *list_new_node(char *path);


struct tree *list_push(struct list **lst, char *path);


struct tree *list_find(const struct list *lst, const char *path);


int list_remove(struct list **lst, const char *path);


void list_clear(struct list **lst);


void list_save(struct list **lst, int fd);


struct tree *hash_insert(struct hash *hsh, char *path);


struct tree *hash_find(const struct hash hsh, const char *path);


int hash_remove(struct hash *hsh, const char *path);


void hash_clear(struct hash *hsh);


void hash_save(struct hash *hsh, int fd);


#endif
