#include "analysis.h"

#include <stdlib.h>
#include <pthread.h>

struct treap *create_node(const int id)
{
	struct treap *ret = (struct treap *)malloc(sizeof(struct treap));
	
	if (!ret) {
		return NULL;
	}
	
	ret->id = id;
	ret->heap_val = rand();
	
	ret->anal = NULL;
	
	ret->parent = NULL;
	ret->chld_left = NULL;
	ret->chld_right = NULL;
	
	return ret;
}


void balance_treap(struct treap **trp)
{
	if ((*trp)->chld_left && (*trp)->heap_val > (*trp)->chld_left->heap_val) {
		if ((*trp)->parent) {
			if ((*trp)->parent->chld_left == *trp) {
				(*trp)->parent->chld_left = (*trp)->chld_left;
			} else {
				(*trp)->parent->chld_right = (*trp)->chld_left;
			}
		}
			
		struct treap *self = *trp;
		struct treap *lft = (*trp)->chld_left;
		struct treap *swtch = (*trp)->chld_left->chld_right;
		
		self->chld_left = swtch;
		lft->chld_right = self;
		
		*trp = lft;
	} else if ((*trp)->chld_right && (*trp)->heap_val > (*trp)->chld_right->heap_val) {
		if ((*trp)->parent) {
			if ((*trp)->parent->chld_left == *trp) {
				(*trp)->parent->chld_left = (*trp)->chld_right;
			} else {
				(*trp)->parent->chld_right = (*trp)->chld_right;
			}
		}
			
		struct treap *self = *trp;
		struct treap *rgt = (*trp)->chld_right;
		struct treap *swtch = (*trp)->chld_right->chld_left;
		
		self->chld_right = swtch;
		rgt->chld_left = self;
		
		*trp = rgt;
	}
}


int insert_treap(struct treap **trp, const int id)
{
	if (!(*trp)) {
		(*trp) = create_node(id);
		if (!(*trp)) {
			return 1;
		}
		return 0;
	}
	
	if (id < (*trp)->id) {
		if ((*trp)->chld_left) {
			if (insert_treap(&((*trp)->chld_left), id)) {
				return 1;
			}
		} else {
			struct treap *new = create_node(id);
			if (!new) {
				return 1;
			}
			
			(*trp)->chld_left = new;
		}
	} else {
		if ((*trp)->chld_right) {
			if (insert_treap(&((*trp)->chld_right), id)) {
				return 1;
			}
		} else {
			struct treap *new = create_node(id);
			if (!new) {
				return 1;
			}
			
			(*trp)->chld_right = new;
		}
	}
	
	balance_treap(trp);
	return 0;
}


void insert_treap(struct treap **trp, const struct treap *node)
{
	if (!(*trp)) {
		(*trp) = node;
		return;
	}
	
	if (id < (*trp)->id) {
		if ((*trp)->chld_left) {
			insert_treap(&((*trp)->chld_left), id);
		} else {
			(*trp)->chld_left = node;
		}
	} else {
		if ((*trp)->chld_right) {
			insert_treap(&((*trp)->chld_right), id);
		} else {
			(*trp)->chld_right = node;
		}
	}
	
	balance_treap(trp);
}


struct treap *get_greatest_smaller(struct treap *trp)
{
	if (!(trp->chld_left)) {
		return NULL;
	}
	
	trp = trp->chld_left;
	while (trp->chld_right) {
		trp = trp->chld_right;
	}
	
	return trp;
}


struct treap *get_smallest_greater(struct treap *trp)
{
	if (!(trp->chld_right)) {
		return NULL;
	}
	
	trp = trp->chld_right;
	while (trp->chld_left) {
		trp = trp->chld_left;
	}
	
	return trp;
}


int remove_treap(struct treap **trp, const int id)
{
	if ((*trp)->id == id) {
		struct treap *placeholder = get_greatest_smaller(*trp);
		if (!placeholder) {
			placeholder = get_smallest_greater(*trp);
			if (!placeholder) {
				()
			}
		}
	}
}
