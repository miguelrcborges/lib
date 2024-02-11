#include "lib/io.h"
#include <stdio.h>
#include <limits.h>
#include <string.h>

bool io_write(usize fd, string s) {
	if (unlikely(s.len > INT_MAX)) {
		return 1;
	}
	return fprintf((FILE *) fd, "%.*s", (int) s.len, s.str) < 0;
}

bool io_read(usize fd, u8 *buf, usize len, usize *written) {
	u8 *end = buf + len;
	char c;
	while (buf != end && (c = fgetc((FILE *)fd)) != EOF) {
		*buf = c;
		buf += 1;
	}
	return ferror((FILE *)fd);
}

bool io_open(string file, u32 mode, usize *fd) {
	if (unlikely(mode >= IO_MODES_COUNT)) {
		return 1;
	}

	static char *mode_lookup[IO_MODES_COUNT] = {
		[IO_READ]   = "rb+",
		[IO_WRITE]  = "wb+", 
		[IO_APPEND] = "ab+",
	};

	if (likely(file.str[file.len] != '\0')) {
		*fd = (usize) fopen((char *)file.str, mode_lookup[mode]);
	} else {
		char zeroed[32768];
		memcpy(zeroed, file.str, file.len);
		zeroed[file.len] = '\0';
		*fd = (usize) fopen(zeroed, mode_lookup[mode]);
	}
	if (unlikely(*fd == 0)) {
		return 1;
	}
	setbuf((FILE *)fd, NULL);
	return 0;
}

bool io_close(usize fd) {
	return fclose((FILE *)fd);
}
