#pragma once

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

typedef struct rbtree *rbtree_t;

rbtree_t rbtree_new(rbtree_alloc_t a, rbtree_free_t f, rbtree_comparer_t c);

enum rbtree_result rbtree_insert(rbtree_t t, void *v);
bool rbtree_delete(struct rbtree *t, void *k);

void * rbtree_find(struct rbtree *t, void *k);
