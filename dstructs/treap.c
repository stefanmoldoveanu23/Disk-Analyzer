#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

#include "treap.h"

struct treap *create_node(const int id, struct analysis *anal)
{
	struct treap *ret = (struct treap *)malloc(sizeof(struct treap));
	if (anal) {
		printf("IN: %s\n", anal->path);
	}
	
	if (!ret) {
		return NULL;
	}
	
	ret->id = id;
	ret->heap_val = rand();
	
	ret->anal = anal;
	
	ret->chld_left = NULL;
	ret->chld_right = NULL;
	
	return ret;
}


void swing_left(struct treap **trp)
{
	struct treap *self = *trp;
	struct treap *lft = (*trp)->chld_left;
	struct treap *swtch = (*trp)->chld_left->chld_right;
		
	*trp = lft;
		
	self->chld_left = swtch;
	lft->chld_right = self;
}


void swing_right(struct treap **trp)
{
	struct treap *self = *trp;
	struct treap *rgt = self->chld_right;
	struct treap *swtch = rgt->chld_left;
	
	*trp = rgt;
	
	self->chld_right = swtch;
	rgt->chld_left = self;
}


int insert_treap_new(struct treap **trp, const int id, struct analysis *anal)
{
	if (!(*trp)) {
		(*trp) = create_node(id, anal);
		if (!(*trp)) {
			return 1;
		}
		return 0;
	}
	
	if (id < (*trp)->id) {
		if ((*trp)->chld_left) {
			if (insert_treap_new(&((*trp)->chld_left), id, anal)) {
				return 1;
			}
		} else {
			struct treap *new = create_node(id, anal);
			if (!new) {
				return 1;
			}
			
			(*trp)->chld_left = new;
		}
	} else {
		if ((*trp)->chld_right) {
			if (insert_treap_new(&((*trp)->chld_right), id, anal)) {
				return 1;
			}
		} else {
			struct treap *new = create_node(id, anal);
			if (!new) {
				return 1;
			}
			
			(*trp)->chld_right = new;
		}
	}
	
	if ((*trp)->chld_left && (*trp)->heap_val > (*trp)->chld_left->heap_val) {
		swing_left(trp);
	} else if ((*trp)->chld_right && (*trp)->heap_val > (*trp)->chld_right->heap_val) {
		swing_right(trp);
	}
	
	return 0;
}


void insert_treap_node(struct treap **trp, struct treap *node)
{
	if (!(*trp)) {
		(*trp) = node;
		return;
	}
	
	if (node->id < (*trp)->id) {
		if ((*trp)->chld_left) {
			insert_treap_node(&((*trp)->chld_left), node);
		} else {
			(*trp)->chld_left = node;
		}
	} else {
		if ((*trp)->chld_right) {
			insert_treap_node(&((*trp)->chld_right), node);
		} else {
			(*trp)->chld_right = node;
		}
	}
	
	if ((*trp)->chld_left && (*trp)->heap_val > (*trp)->chld_left->heap_val) {
		swing_left(trp);
	} else if ((*trp)->chld_right && (*trp)->heap_val > (*trp)->chld_right->heap_val) {
		swing_right(trp);
	}
}


int find_treap(struct treap *trp, const int id, struct analysis **anal) {
	if (!trp) {
		if (anal) {
			*anal = NULL;
		}
		
		return 0;
	}
	
	if (trp->id == id) {
		if (anal) {
			*anal = trp->anal;
		}
		
		return 1;
	}
	
	if (trp->id < id) {
		return find_treap(trp->chld_right, id, anal);
	}
	
	return find_treap(trp->chld_left, id, anal);
}


struct treap *extract_treap(struct treap **trp, const int id)
{
	if (!(*trp)) {
		return NULL;
	}
	
	if ((*trp)->id == id) {
		struct treap *ret = NULL;
		
		if (!((*trp)->chld_left) && !((*trp)->chld_right)) {
			ret = *trp;
			*trp = NULL;
		} else if (!((*trp)->chld_left)) {
			ret = *trp;
			*trp = (*trp)->chld_right;
		} else if (!((*trp)->chld_right)) {
			ret = *trp;
			*trp = (*trp)->chld_left;
		} else {
			if ((*trp)->chld_left->heap_val < (*trp)->chld_right->heap_val) {
				swing_right(trp);
			} else {
				swing_left(trp);
			}
		}
		
		if (ret) {
			return ret;
		}

	}
	
	if ((*trp)->id < id) {
		return extract_treap(&((*trp)->chld_right), id);
	} else {
		return extract_treap(&((*trp)->chld_left), id);
	}
}


int remove_treap(struct treap **trp, const int id)
{
	struct treap *to_delete = extract_treap(trp, id);
	printf("a\n");
	
	if (to_delete) {
		printf("b\n");
		if (to_delete->anal) {
			free(to_delete->anal->path);
			free(to_delete->anal);
		}
		free(to_delete);
		return 0;
	}
	
	return 1;
}


void clear_treap(struct treap **trp)
{
	while (*trp) {
		remove_treap(trp, (*trp)->id);
	}
}


void save_treap(struct treap **trp, int fd)
{
	while (*trp) {
		printf("OUT: %s\n", (*trp)->anal->path);
		analysis_write((*trp)->id, (*trp)->anal, fd);
		remove_treap(trp, (*trp)->id);
	}
}
