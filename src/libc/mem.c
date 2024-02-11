#include "lib.h"

#include <stdlib.h>
#include <string.h>

SafePointer mem_reserve(usize size) {
	SafePointer r;
	r._ptr = malloc(size);
	return r;
}

SafePointer mem_rescommit(usize size) {
	SafePointer r;
	r._ptr = calloc(size, 1);
	return r;
}

bool mem_commit(void *ptr, usize size) {
	memset(ptr, 0, size);
	return 0;
}

bool mem_decommit(void *ptr, usize size) {
	return 0;
}

bool mem_release(void *ptr, usize size) {
	free(ptr);
	return 0;
}
