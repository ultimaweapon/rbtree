#include "rbtree.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int destroyed;

static bool insert_and_delete(rbtree_t t)
{
	rbtree_node_t n;

	// create node
	n = rbtree_node_new(t, "test");
	if (!n) {
		fprintf(stderr, "Node creation failed.\n");
		return false;
	}

	// insert.
	if (!rbtree_insert(t, n)) {
		fprintf(stderr, "Insertion failed.\n");
		rbtree_node_free(n);
		return false;
	}

	if (rbtree_size(t) != 1) {
		fprintf(stderr, "Tree size is not 1 after insertion.\n");
		return false;
	}

	// delete.
	if (!rbtree_delete(t, "test")) {
		fprintf(stderr, "Deletion failed.\n");
		return false;
	}

	if (rbtree_size(t) != 0) {
		fprintf(stderr, "Tree size is not 0 after deletion.\n");
		return false;
	}

	return true;
}

static bool test(rbtree_t t)
{
	if (!insert_and_delete(t)) {
		return false;
	}

	if (destroyed != 1) {
		fprintf(stderr, "Value was not destroyed.\n");
		return false;
	}

	if (!insert_and_delete(t)) {
		return false;
	}

	if (destroyed != 2) {
		fprintf(stderr, "Value was not destroyed.\n");
		return false;
	}

	return true;
}

static int compare(const void *first, const void *second)
{
	return strcmp((const char *)first, (const char *)second);
}

static size_t nodesize(const void *value)
{
	return strlen((const char *)value) + 1;
}

static void destroy(void *value)
{
	if (strcmp((const char *)value, "test") == 0) {
		destroyed++;
	}
}

int main(int argc, char *argv[])
{
	struct rbtree_init init;
	rbtree_t t;
	int res;

	// create a new tree.
	memset(&init, 0, sizeof(init));

	init.alloc = malloc;
	init.free = free;
	init.compare = compare;
	init.size = nodesize;
	init.destroy = destroy;

	t = rbtree_new(&init);
	if (!t) {
		fprintf(stderr, "No available memory to create a tree.\n");
		return EXIT_FAILURE;
	}

	res = test(t) ? EXIT_SUCCESS : EXIT_FAILURE;;

	// clean up.
	rbtree_free(t);

	return res;
}
