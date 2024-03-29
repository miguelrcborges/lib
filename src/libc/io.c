#include "lib.h"
#include <stdio.h>
#include <limits.h>
#include <string.h>

void io_write(usize fd, string s) {
	usize written = 0;
	while (written < s.len) {
		int to_write = min(INT_MAX, s.len - written);
		int tmp = fprintf((FILE *)fd, "%.*s", to_write, s.str + written);
		if (unlikely(tmp < 0)) {
			fprintf(stderr, "Failed to write to file");
			die(1);
		}
		written += tmp;
	}
}

usize io_read(usize fd, u8 *buf, usize len) {
	usize read = 0;
	u32 last_count;
	while (read < len) {
		u32 read_amount = min(last_count - read, UINT32_MAX);
		u32 tmp = fread(buf, 1, len, (FILE*)fd);
		if (unlikely(tmp < 0)) {
			io_write(getStdErr(), str("Failed to write to file.\n"));
			die(1);
		}
		read += tmp;
		if (last_count < read_amount) {
			break;
		}
	}
	return read;
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
