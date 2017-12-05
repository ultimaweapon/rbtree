#include <rbtree.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int comparer(const void *first, const void *second);
void destroyer(void *value);
size_t sizer(const void *value);
bool test(rbtree_t t);

int main(int argc, char *argv[])
{
	struct rbtree_init init;
	rbtree_t t;
	int res;

	// create a new tree.
	memset(&init, 0, sizeof(init));
	init.alloc = malloc;
	init.compare = comparer;
	init.destroy = destroyer;
	init.free = free;
	init.size = sizer;

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
