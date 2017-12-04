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
typedef int (*rbtree_comparer_t) (const void *first, const void *second);
typedef size_t (*rbtree_size_t) (const void *value);
typedef void (*rbtree_destroy_t) (void *value);

struct rbtree;
struct rbtree_node;

struct rbtree_init {
    rbtree_alloc_t alloc;
    rbtree_free_t free;
    rbtree_comparer_t compare;
    rbtree_size_t size;
    rbtree_destroy_t destroy;
};

typedef struct rbtree *rbtree_t;
typedef struct rbtree_node *rbtree_node_t;

rbtree_t rbtree_new(const struct rbtree_init *i);

enum rbtree_result rbtree_insert(rbtree_t t, const void *v);
bool rbtree_delete(rbtree_t t, const void *v);

void * rbtree_find(rbtree_t t, const void *v);

#endif
