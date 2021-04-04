#ifndef RBTREE_H
#define RBTREE_H

#include <stdbool.h>
#include <stddef.h>

struct rbtree;
struct rbtree_node;

typedef struct rbtree *rbtree_t;
typedef struct rbtree_node *rbtree_node_t;

typedef void * (*rbtree_alloc_t) (size_t bytes);
typedef void (*rbtree_free_t) (void *ptr);
typedef int (*rbtree_comparer_t) (const void *first, const void *second);
typedef size_t (*rbtree_size_t) (const void *value);
typedef void (*rbtree_destroy_t) (void *value);
typedef void (*rbtree_enum_t) (rbtree_node_t node, void *context);

#ifdef RBTREE_DEBUG
typedef const char * (*rbtree_string_t) (const void *value);
#endif

struct rbtree_init {
	rbtree_alloc_t alloc;
	rbtree_free_t free;
	rbtree_comparer_t compare;
	rbtree_size_t size;
	rbtree_destroy_t destroy;
#ifdef RBTREE_DEBUG
	rbtree_string_t to_string;
#endif
};

rbtree_t rbtree_new(const struct rbtree_init *i);
void rbtree_free(rbtree_t t);

bool rbtree_insert(rbtree_t t, rbtree_node_t n);
bool rbtree_delete(rbtree_t t, const void *v);

size_t rbtree_size(rbtree_t t);
rbtree_node_t rbtree_find(rbtree_t t, const void *v);
void rbtree_enum(rbtree_t t, rbtree_enum_t h, void *c);

rbtree_node_t rbtree_node_new(rbtree_t t, const void *v);
void rbtree_node_free(rbtree_node_t n);

void * rbtree_node_value(rbtree_node_t n);
bool rbtree_node_is_clean(rbtree_node_t n);

#endif
