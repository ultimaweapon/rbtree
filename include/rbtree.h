#ifndef RBTREE_H
#define RBTREE_H

#include <stdbool.h>
#include <stddef.h>

enum rbtree_result {
	rbtree_success,
	rbtree_nomem,
	rbtree_exists
};

typedef void * (*rbtree_alloc_t) (size_t bytes);
typedef void (*rbtree_free_t) (void *ptr);
typedef int (*rbtree_comparer_t) (void *first, void *second);

struct rbtree;
struct rbtree_node;

typedef struct rbtree *rbtree_t;
typedef struct rbtree_node *rbtree_node_t;

rbtree_t rbtree_new(rbtree_alloc_t a, rbtree_free_t f, rbtree_comparer_t c);

enum rbtree_result rbtree_insert(rbtree_t t, void *v);
bool rbtree_delete(struct rbtree *t, void *k);

void * rbtree_find(struct rbtree *t, void *k);

#endif
