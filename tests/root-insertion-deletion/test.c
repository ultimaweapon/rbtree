#include "../runner.c"

static int destroyed;

static bool insert_and_delete(rbtree_t t)
{
	enum rbtree_result r;

	// insert.
	r = rbtree_insert(t, "test");
	if (r != rbtree_success) {
		fprintf(stderr, "Insertion failed with code %d.\n", r);
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

int comparer(const void *first, const void *second)
{
	return strcmp((const char *)first, (const char *)second);
}

void destroyer(void *value)
{
	if (strcmp((const char *)value, "test") == 0) {
		destroyed++;
	}
}

size_t sizer(const void *value)
{
	return strlen((const char *)value) + 1;
}

bool test(rbtree_t t)
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
