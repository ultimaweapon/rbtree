#pragma once

#include <stdbool.h>

typedef int (*rbtree_comparer_t) (void *first, void *second);
typedef void (*rbtree_free_t) (void *v);

struct rbtree;

struct rbtree * rbtree_new(rbtree_comparer_t c, rbtree_free_t f);

bool rbtree_insert(struct rbtree *t, void *v);
bool rbtree_delete(struct rbtree *t, void *k);

void * rbtree_find(struct rbtree *t, void *k);
