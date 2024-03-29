#include "lib.h"

#include <stdlib.h>
#include <string.h>

void *mem_reserve(usize size) {
	void *p = malloc(size);
	if (unlikely(p == NULL)) {
		io_write(getStdErr(), str("Out of memory.\n"));
		die(1);
	}
	return p;
}

void *mem_rescommit(usize size) {
	void *p = calloc(size, 1);
	if (unlikely(p == NULL)) {
		io_write(getStdErr(), str("Out of memory.\n"));
		die(1);
	}
	return p;
}

void mem_commit(void *ptr, usize size) {
	memset(ptr, 0, size);
	return;
}

void mem_decommit(void *ptr, usize size) {
	return;
}

void mem_release(void *ptr, usize size) {
	free(ptr);
	return;
}

usize mem_getPageSize(void) {
	return 1;
}

void die(usize code) {
	exit(code);
}
