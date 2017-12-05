#include <rbtree.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int compare(const void *first, const void *second)
{
	int f = *(const int *)first;
	int s = *(const int *)second;
	return f - s;
}

static void destroy(void *value)
{
	printf("Destroying value %d.\n", *(int *)value);
}

static size_t size(const void *value)
{
	return sizeof(int);
}

static const char * to_string(const void *value)
{
	static char buf[16];
	sprintf(buf, "%d", *(const int *)value);
	return buf;
}

static rbtree_t create_tree(void)
{
	struct rbtree_init i;

	memset(&i, 0, sizeof(i));
	i.alloc = malloc;
	i.compare = compare;
	i.destroy = destroy;
	i.free = free;
	i.size = size;
	i.to_string = to_string;

	return rbtree_new(&i);
}

int main(int argc, char *argv[])
{
	rbtree_t t;

	// create tree.
	t = create_tree();
	if (!t) {
		fprintf(stderr, "Insufficient memory for creating a tree.\n");
		return EXIT_FAILURE;
	}

	// set stdout to non-buffering.
	setvbuf(stdout, NULL, _IONBF, 0);

	// REPL.
	for (;;) {
		char op[8];
		int v;
		enum rbtree_result r;

		// read input.
		printf("rbtree: ");

		switch (scanf("%s %d", op, &v)) {
		case 0:
		case 1:
			printf("Usage: (add|del) NUM\n");
			continue;
		case EOF:
			goto end;
		}

		if (strcmp(op, "add") == 0) {
			// insert value.
			r = rbtree_insert(t, &v);
			switch (r) {
			case rbtree_success:
			case rbtree_exists:
				break;
			case rbtree_nomem:
				printf("Insufficient memory for creating a new node.\n");
				break;
			default:
				printf("rbtree_insert() return an unknown result %d.\n", r);
			}
		} else if (strcmp(op, "del") == 0) {
			rbtree_delete(t, &v);
		} else {
			printf("Unknown operation: %s.\n", op);
		}
	}

	// clean up.
end:
	rbtree_free(t);

	return EXIT_SUCCESS;
}
