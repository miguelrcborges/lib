#include "lib.h"
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#include "common.c"

void io_write(usize fd, string s) {
	usize r = fwrite(s.str, 1, s.len, (FILE *)fd);
	if (r != s.len) {
		fprintf(stderr, "Failed to write to file");
		die(1);
	}
}

usize io_read(usize fd, u8 *buf, usize len) {
	usize r = fread(buf, 1, len, (FILE *)fd);
	if (r == 0 && ferror((FILE *)fd)) {
		fprintf(stderr, "Failed to read file");
		die(1);
	}
	return r;
}

usize io_open(string file, u32 mode) {
	usize fd;
	if (unlikely(mode >= IO_MODES_COUNT)) {
		io_write(getStdErr(), str("Invalid mode to open file.\n"));
		die(1);
	}

	static char *mode_lookup[IO_MODES_COUNT] = {
		[IO_READ]   = "rb",
		[IO_WRITE]  = "wb", 
		[IO_APPEND] = "ab",
	};

	if (likely(file.str[file.len] == '\0')) {
		fd = (usize) fopen((char *)file.str, mode_lookup[mode]);
	} else {
		char zeroed[32768];
		memcpy(zeroed, file.str, file.len);
		zeroed[file.len] = '\0';
		fd = (usize) fopen(zeroed, mode_lookup[mode]);
	}
	if (unlikely(fd == 0)) {
		io_write(getStdErr(), str("Failed to open file.\n"));
		die(1);
	}
	setbuf((FILE *)fd, NULL);
	return fd;
}

void io_close(usize fd) {
	if (unlikely(fclose((FILE *)fd))) {
		io_write(getStdErr(), str("Failed to close file.\n"));
		die(1);
	}
}

usize io_len(usize fd) {
	fpos_t init;
	FILE *f = (FILE *)fd;
	if (unlikely(fgetpos(f, &init))) {
		io_write(getStdErr(), str("Failed to get file length.\n"));
		die(1);
	}
	if (unlikely(fseek(f, 0, SEEK_END))) {
		io_write(getStdErr(), str("Failed to get file length.\n"));
		die(1);
	};
	usize len = ftell(f);
	if (unlikely(len == (usize)-1)) {
		io_write(getStdErr(), str("Failed to get file length.\n"));
		die(1);
	}
	if (unlikely(fsetpos(f, &init))) {
		io_write(getStdErr(), str("Failed to get file length.\n"));
		die(1);
	};
	return len;
}


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
