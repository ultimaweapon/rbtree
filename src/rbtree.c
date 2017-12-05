#include "rbtree.h"

#include <stdbool.h>
#include <stddef.h>
#ifdef RBTREE_DEBUG
#include <stdio.h>
#endif
#include <string.h>

enum nodetype {
	node_black,
	node_red
};

struct rbtree_node {
	enum nodetype type;
	struct rbtree *owner;
	struct rbtree_node *parent;
	struct rbtree_node *left;
	struct rbtree_node *right;
};

struct rbtree {
	size_t node;
	rbtree_alloc_t alloc;
	rbtree_free_t free;
	rbtree_comparer_t compare;
	rbtree_size_t size;
	rbtree_destroy_t destroy;
#ifdef RBTREE_DEBUG
	rbtree_string_t to_string;
#endif
	struct rbtree_node *root;
};

static void rebalance_insertion(struct rbtree *t, struct rbtree_node *i);

#ifdef RBTREE_DEBUG
static const char * node_color(struct rbtree_node *n)
{
	if (n->type == node_black) {
		return "black";
	} else {
		return "red";
	}
}
#endif

static void swap_color(struct rbtree_node *first, struct rbtree_node *second)
{
	enum nodetype ft, st;

	ft = first->type;
	st = second->type;

	first->type = st;
	second->type = ft;
}

static void swap_node(struct rbtree_node *f, struct rbtree_node *s)
{
	enum nodetype ft = f->type;
	struct rbtree_node *fp = f->parent;
	struct rbtree_node *fl = f->left;
	struct rbtree_node *fr = f->right;

	f->type = s->type;
	f->parent = s->parent;
	f->left = s->left;
	f->right = s->right;

	s->type = ft;
	s->parent = fp;
	s->left = fl;
	s->right = fr;
}

static void reverse_color(struct rbtree_node *n)
{
	n->type = (n->type == node_black) ? node_red : node_black;
}

static void enumerate_node(struct rbtree_node *n, rbtree_enum_t h, void *ctx)
{
	struct rbtree_node *l, *r;

	if (!n) {
		return;
	}

	l = n->left;
	r = n->right;

	enumerate_node(l, h, ctx);
	h(n, ctx);
	enumerate_node(r, h, ctx);
}

static void rotate_left(struct rbtree_node *n)
{
	struct rbtree_node *p = n->parent;
	struct rbtree_node *r = n->right;

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

static void rotate_right(struct rbtree_node *n)
{
	struct rbtree_node *p = n->parent;
	struct rbtree_node *l = n->left;

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

static void rotate(struct rbtree *t, struct rbtree_node *i, struct rbtree_node *p, struct rbtree_node *g, struct rbtree_node *u)
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
			rebalance_insertion(t, p);
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
			rebalance_insertion(t, p);
		}
	}
}

static void rebalance_insertion(struct rbtree *t, struct rbtree_node *i)
{
	// inserted node is always red.
	struct rbtree_node *p = i->parent;

	if (!p) {
		// target node is the root node.
#ifdef RBTREE_DEBUG
		printf("Inserting node is a root node, paint it to black.\n");
#endif
		i->type = node_black;
	} else if (p->type == node_red) {
		// current node and parent node are red node. all children of red node
		// must be black. we need to fix this. fixing method is depend on uncle node.
		struct rbtree_node *g, *u;

		// there is a grand parent for sure since parent is red.
		g = p->parent;

		// if parent is on the left of grand parent, that mean uncle will be on
		// the right and wise versa.
		u = (p == g->left) ? g->right : g->left;

		if (!u || u->type == node_black) {
			// uncle is black.
			rotate(t, i, p, g, u);
		} else {
			// uncle is red. both uncle and parent is red, repaint its to black.
			p->type = node_black;
			u->type = node_black;
			g->type = node_red; // grand parent will always black since all children are red.
			rebalance_insertion(t, g); // grand parent might violate rules, so try to repair it.
		}
	} else {
		// there is nothing to do if parent node is black.
#ifdef RBTREE_DEBUG
		printf("Parent node is black, target node to re-balance is %s.\n", node_color(i));
#endif
	}
}

// all parameters are never null.
// s will always black.
static void rotate_sibling_with_red_child(struct rbtree_node *p, struct rbtree_node *s)
{
	if (s == p->left) {
		// sibling is on the left.
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
static void resolve_double_black(struct rbtree_node *p, struct rbtree_node *n)
{
	struct rbtree_node *s;

	if (!p) {
		// n is a new root.
		return;
	}

	// sibling is never null since the deleted node is black.
	// that mean it have at least one black on the other side.
	s = (n == p->left) ? p->right : p->left;

	if (s->type == node_black) {
		// sibling is black.
		struct rbtree_node *sl, *sr;

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
static void rebalance_deletion(struct rbtree_node *p, struct rbtree_node *n, struct rbtree_node *c)
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
static void free_node(struct rbtree *t, struct rbtree_node *n)
{
	struct rbtree_node *p, *c;

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

	// re-balancing tree before freeing node.
	rebalance_deletion(p, n, c);
	rbtree_node_free(n);
}

static void delete_node(struct rbtree *t, struct rbtree_node *n)
{
	if (n->left && n->right) {
		// find the largest node on the left side.
		struct rbtree_node *max = n->left;

		while (max->right) {
			max = max->right;
		}

		// move the value of largest node to target node.
		swap_node(n, max);
	}

	free_node(t, n);
}

static void destroy_node(rbtree_node_t node, void *context)
{
	struct rbtree *t = context;

	if (t->destroy) {
#ifdef RBTREE_DEBUG
		printf("destroying data on node %s\n", t->to_string(&node[1]));
#endif
		t->destroy(&node[1]);
	}

#ifdef RBTREE_DEBUG
	printf("freeing memory for node %s", t->to_string(&node[1]));
#endif
	t->free(node);
}

rbtree_t rbtree_new(const struct rbtree_init *i)
{
	struct rbtree *t;

	// setup data.
	t = i->alloc(sizeof(t[0]));
	if (!t) {
		return NULL;
	}

	memset(t, 0, sizeof(t[0]));
	t->alloc = i->alloc;
	t->free = i->free;
	t->compare = i->compare;
	t->size = i->size;
	t->destroy = i->destroy;

#ifdef RBTREE_DEBUG
	t->to_string = i->to_string;
#endif

	return t;
}

void rbtree_free(rbtree_t t)
{
	rbtree_enum(t, destroy_node, t);
	t->free(t);
}

bool rbtree_insert(rbtree_t t, rbtree_node_t n)
{
	struct rbtree_node *c;

	// check if node is clean.
	if (!rbtree_node_is_clean(n)) {
		return false;
	}

	// find a leaf to insert.
	c = t->root;

	if (!c) {
		t->root = n;
	} else {
		for (;;) {
			int r = t->compare(rbtree_node_value(n), rbtree_node_value(c));

			if (r > 0) {
				if (c->right) {
					c = c->right;
					continue;
				}
				c->right = n;
			} else if (r < 0) {
				if (c->left) {
					c = c->left;
					continue;
				}
				c->left = n;
			} else {
				return false;
			}

			n->parent = c;
			break;
		}
	}

	// fix up.
	rebalance_insertion(t, n);
	t->node++;

	return true;
}

bool rbtree_delete(rbtree_t t, const void *v)
{
	struct rbtree_node *current = t->root;

	while (current) {
		int cmp = t->compare(v, &current[1]);

		if (cmp > 0) {
			current = current->right;
		} else if (cmp < 0) {
			current = current->left;
		} else {
			delete_node(t, current);
			t->node--;
			return true;
		}
	}

	return false;
}

size_t rbtree_size(rbtree_t t)
{
	return t->node;
}

rbtree_node_t rbtree_find(rbtree_t t, const void *v)
{
	struct rbtree_node *current = t->root;

	while (current) {
		int cmp = t->compare(v, &current[1]);

		if (cmp > 0) {
			current = current->right;
		} else if (cmp < 0) {
			current = current->left;
		} else {
			return current;
		}
	}

	return NULL;
}

void rbtree_enum(rbtree_t t, rbtree_enum_t h, void *ctx)
{
	enumerate_node(t->root, h, ctx);
}

rbtree_node_t rbtree_node_new(rbtree_t t, const void *v)
{
	size_t vs;
	struct rbtree_node *n;

	// allocate node.
	vs = t->size(v);
	n = t->alloc(sizeof(n[0]) + vs);
	if (!n) {
		return NULL;
	}

	// initialize node.
	memset(n, 0, sizeof(n[0]));
	memcpy(&n[1], v, vs);

	n->owner = t;
	n->type = node_red;

	return n;
}

void rbtree_node_free(rbtree_node_t n)
{
	if (n->owner->destroy) {
		n->owner->destroy(rbtree_node_value(n));
	}

	n->owner->free(n);
}

void * rbtree_node_value(rbtree_node_t n)
{
	return &n[1];
}

bool rbtree_node_is_clean(rbtree_node_t n)
{
	return !n->parent && !n->left && !n->right && (n->type == node_red);
}
