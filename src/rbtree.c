#include "rbtree.h"

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum nodetype {
	node_black,
	node_red
};

struct node {
	enum nodetype type;
	void *value;
	struct node *parent;
	struct node *left;
	struct node *right;
};

struct rbtree {
	rbtree_comparer_t comparer;
	rbtree_free_t vfree;
	struct node *root;
};

static void rebalance_insertion(struct node *i);

static void swap_color(struct node *first, struct node *second)
{
	enum nodetype ft, st;

	ft = first->type;
	st = second->type;

	first->type = st;
	second->type = ft;
}

static void reverse_color(struct node *n)
{
	n->type = (n->type == node_black) ? node_red : node_black;
}

static void rotate_left(struct node *n)
{
	struct node *p = n->parent;
	struct node *r = n->right;

	if (!r) {
		// no right node for moving to the top.
		return;
	}

	// move right node to the top.
	r->parent = p;
	if (p) {
		if (p->left == n) {
			p->left = r;
		} else {
			p->right = r;
		}
	}

	// move left node of the right node to the right of target node.
	n->right = r->left;
	if (r->left) {
		r->left->parent = n;
	}

	// move target node to the left.
	r->left = n;
	n->parent = r;
}

static void rotate_right(struct node *n)
{
	struct node *p = n->parent;
	struct node *l = n->left;

	if (!l) {
		// no left node for moving to the top.
		return;
	}

	// move left node to the top.
	l->parent = p;
	if (p) {
		if (p->left == n) {
			p->left = l;
		} else {
			p->right = l;
		}
	}

	// move right node of the left to the left of target node.
	n->left = l->right;
	if (l->right) {
		l->right->parent = n;
	}

	// move target node to the right.
	l->right = n;
	n->parent = l;
}

static void rotate(struct node *i, struct node *p, struct node *g, struct node *u)
{
	// the only value that can be null is uncle. the uncle is always black when
	// we need to rotate.
	if (p == g->left) {
		if (i == p->left) {
			// parent and inserted node is on the left.
			// rotate grand parent to the right.
			rotate_right(g);
			swap_color(p, g);
		} else {
			// parent is on the left but inserted node is on the right.
			// rotate parent to the left. then repair parent.
			rotate_left(p);
			rebalance_insertion(p);
		}
	} else {
		if (i == p->right) {
			// both parent and inserted node is on the right.
			// rotate grand parent to the left and swap color with parent.
			rotate_left(g);
			swap_color(p, g);
		} else {
			// parent is on the right but inserted node is on the left.
			// rotate parent to the right. then try to repair parent.
			rotate_right(p);
			rebalance_insertion(p);
		}
	}
}

static void rebalance_insertion(struct node *i)
{
	// inserted node is always red.
	struct node *p = i->parent;

	if (!p) {
		// target node is the root node.
		i->type = node_black;
	} else if (p->type == node_red) {
		// current node and parent node are red node. all children of red node
		// must be black. we need to fix this. fixing method is depend on uncle node.
		struct node *g, *u;

		// there is a grand parent for sure since parent is red.
		g = p->parent;

		// if parent is on the left of grand parent, that mean uncle will be on
		// the right and wise versa.
		u = (p == g->left) ? g->right : g->left;

		if (!u || u->type == node_black) {
			// uncle is black.
			rotate(i, p, g, u);
		} else {
			// uncle is red. both uncle and parent is red, repaint its to black.
			p->type = node_black;
			u->type = node_black;
			g->type = node_red; // grand parent will always black since all children are red.
			rebalance_insertion(g); // grand parent might violate rules, so try to repair it.
		}
	} else {
		// there is nothing to do if parent node is black.
	}
}

// all parameters are never null.
// s will always black.
static void rotate_sibling_with_red_child(struct node *p, struct node *s)
{
	if (s == p->left) {
		// sibgling is on the left.
		if (s->left && s->left->type == node_red) {
			// red child of sibling is on the left or both children are red.
			rotate_right(p);
			swap_color(p, s);
			s->left->type = node_black;
		} else {
			// red child of sibling is on the right.
			rotate_left(s);
			swap_color(s, s->right);
			rotate_sibling_with_red_child(p, s->right);
		}
	} else {
		// sibling is on the right.
		if (s->right && s->right->type == node_red) {
			// red child of sibling is on the right or both children are red.
			rotate_left(p);
			swap_color(p, s);
			s->right->type = node_black;
		} else {
			// red child of sibling is on the left.
			rotate_right(s);
			swap_color(s, s->left);
			rotate_sibling_with_red_child(p, s->left);
		}
	}
}

// n is double black, so it might be null. p might also be null in case of n became root.
static void resolve_double_black(struct node *p, struct node *n)
{
	struct node *s;

	if (!p) {
		// n is a new root.
		return;
	}

	// sibling is never null since the deleted node is black.
	// that mean it have at least one black on the other side.
	s = (n == p->left) ? p->right : p->left;

	if (s->type == node_black) {
		// sibling is black.
		struct node *sl, *sr;

		sl = s->left;
		sr = s->right;

		if ((sl && sl->type == node_red) || (sr && sr->type == node_red)) {
			// at least one of sibling's children are red.
			rotate_sibling_with_red_child(p, s);
		} else {
			// both children of sibling are black.
			s->type = node_red;

			if (p->type == node_red) {
				p->type = node_black;
			} else {
				resolve_double_black(p->parent, p);
			}
		}
	} else {
		// sibling is red.
		if (s == p->left) {
			// sibling is on the left.
			rotate_right(p);
		} else {
			// sibling is on the right.
			rotate_left(p);
		}
		reverse_color(p);
		reverse_color(s);

		resolve_double_black(p, n);
	}
}

// children might be null in case of target node have no children.
static void rebalance_deletion(struct node *p, struct node *n, struct node *c)
{
	if ((n->type == node_red) || (c && c->type == node_red)) {
		// either target node or its children is red.
		if (c) {
			c->type = node_black;
		}
	} else {
		// both target node and its child node are black.
		// now children will become double black.
		resolve_double_black(p, c);
	}
}

// a node that can be free must either have one children or no children.
static void free_node(struct rbtree *t, struct node *n)
{
	struct node *p, *c;

	p = n->parent;
	c = n->left ? n->left : n->right;

	// replace target node with its children.
	if (p) {
		if (n == p->left) {
			p->left = c;
		} else {
			p->right = c;
		}
	} else {
		// target node is root node.
		t->root = c;
	}

	if (c) {
		c->parent = p;
	}

	// re-balance tree before freeing node.
	rebalance_deletion(p, n, c);

	// free target node.
	if (n->value) {
		// value might already moved to another node, so check before freeing it.
		t->vfree(n->value);
	}
	free(n);
}

static void delete_node(struct rbtree *t, struct node *n)
{
	if (n->left && n->right) {
		// find the largest node on the left side.
		struct node *max = n->left;

		while (max->right) {
			max = max->right;
		}

		// move the value of largest node to target node.
		t->vfree(n->value);
		n->value = max->value;
		max->value = NULL;

		free_node(t, max);
	} else {
		free_node(t, n);
	}
}

struct rbtree * rbtree_new(rbtree_comparer_t c, rbtree_free_t f)
{
	struct rbtree *t;

	// setup data.
	t = calloc(1, sizeof(t[0]));
	if (!t) {
		const char *reason = strerror(errno);
		fprintf(stderr, "Failed to allocated data for Red-black tree: %s.\n", reason);
		return NULL;
	}

	t->comparer = c;
	t->vfree = f;

	return t;
}

bool rbtree_insert(struct rbtree *t, void *v)
{
	struct node *n;

	// allocate node.
	n = calloc(1, sizeof(n[0]));
	if (!n) {
		const char *reason = strerror(errno);
		fprintf(stderr, "Failed to allocated node for Red-black tree: %s.\n", reason);
		return false;
	}

	n->type = node_red;
	n->value = v;

	// insert node.
	if (!t->root) {
		t->root = n;
	} else {
		struct node *current = t->root;

		for (;;) {
			int cmp = t->comparer(v, current->value);

			if (cmp > 0) {
				if (!current->right) {
					n->type = node_red;
					n->parent = current;
					current->right = n;
					break;
				}
				current = current->right;
			} else if (cmp < 0) {
				if (!current->left) {
					n->type = node_red;
					n->parent = current;
					current->left = n;
					break;
				}
				current = current->left;
			} else {
				return false;
			}
		}
	}

	rebalance_insertion(n);

	return true;
}

bool rbtree_delete(struct rbtree *t, void *k)
{
	struct node *current = t->root;

	while (current) {
		int cmp = t->comparer(k, current->value);

		if (cmp > 0) {
			current = current->right;
		} else if (cmp < 0) {
			current = current->left;
		} else {
			delete_node(t, current);
			return true;
		}
	}

	return false;
}

void * rbtree_find(struct rbtree *t, void *k)
{
	struct node *current = t->root;

	while (current) {
		int cmp = t->comparer(k, current->value);

		if (cmp > 0) {
			current = current->right;
		} else if (cmp < 0) {
			current = current->left;
		} else {
			return current->value;
		}
	}

	return NULL;
}
