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

struct treap *treap_create_node(const int id, struct analysis *anal);


int treap_insert_new(struct treap **trp, const int id, struct analysis *anal);


void treap_insert_node(struct treap **trp, struct treap *node);


int treap_find(struct treap *trp, const int id, struct analysis **anal);


int treap_remove(struct treap **trp, const int id);


struct treap *treap_extract(struct treap **trp, const int id);


void treap_clear(struct treap **trp);


void treap_save(struct treap **trp, int fd);

#endif
