#ifndef TREAP_H
#define TREAP_H

#include "analysis.h"

struct treap{
	int id;
	int heap_val;
	
	struct analysis *anal;
	
	struct treap *chld_left;
	struct treap *chld_right;
};

struct treap *create_node(const int id, struct analysis *anal);


int insert_treap_new(struct treap **trp, const int id, struct analysis *anal);


void insert_treap_node(struct treap **trp, struct treap *node);


int find_treap(struct treap *trp, const int id, struct analysis **anal);


int remove_treap(struct treap **trp, const int id);


struct treap *extract_treap(struct treap **trp, const int id);


void clear_treap(struct treap **trp);


void save_treap(struct treap **trp, int fd);

#endif
