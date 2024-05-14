#include "lib.h"

int __open_syscall(const u8 *name, i32 flags, usize modes);
usize __write_syscall(int fd, const u8 *buf, size_t count);
usize __read_syscall(int fd, const u8 *buf, size_t count);
int __close_syscall(int fd);

// sizeof struct stat is 144
struct stat {
	u8 lpad[48];
	usize len;
	u8 rpad[144-48-8];
};
int __fstat_syscall(int fd, struct stat *s);

usize io_open(string file, u32 mode) {
	int fd;
	if (unlikely(mode >= IO_MODES_COUNT)) {
		io_write(2, str("Invalid mode to open file.\n"));
		die(1);
	}

	static i32 flags_lookup[IO_MODES_COUNT] = {
		[IO_READ]   = 0,
		[IO_WRITE]  = 1101, 
		[IO_APPEND] = 2002,
	};

	static i32 modes_lookup[IO_MODES_COUNT] = {
		[IO_READ]   = 0,
		[IO_WRITE]  = 0644, 
		[IO_APPEND] = 0,
	};

	if (likely(file.str[file.len] == '\0')) {
		fd = __open_syscall(file.str, flags_lookup[mode], modes_lookup[mode]);
	} else {
		u8 zeroed[4097];
		usize i = 0;
		for (; i < file.len; i++) {
			zeroed[i] = file.str[i];
		} 
		zeroed[i] = '\0';
		fd = __open_syscall(file.str, flags_lookup[mode], modes_lookup[mode]);
	}
	if (unlikely(fd < 0)) {
		io_write(2, str("Failed to open file.\n"));
		die(1);
	}
	return fd;
}

void io_write(usize fd, string s) {
	usize written = 0;
	while (written < s.len) {
		// SSIZE_MAX is the max implementation defined. Linux will cap at 0x7ffff000
		usize write_amount = min(s.len - written, ((usize)-1^((usize)1 << 63)));
		usize last_count = __write_syscall(fd, s.str+written, write_amount);
		if (unlikely(write_amount == (usize)-1)) {
			#define ex(x) (u8*)x, sizeof(x)-1
			__write_syscall(2, ex("Failed to write to file.\n"));
			die(1);
		}
		written += last_count;
	}
}

usize io_read(usize fd, u8 *buf, usize len) {
	usize read = 0;
	while (read < len) {
		// SSIZE_MAX is the max implementation defined. Linux will cap at 0x7ffff000
		usize read_amount = min(len - read, ((usize)-1^((usize)1 << 63)));
		usize last_count = __read_syscall(fd, buf+read, read_amount);
		if (unlikely(read_amount == (usize)-1)) {
			io_write(2, str("Failed to read file.\n"));
			die(1);
		}
		read += last_count;
		if (last_count < read_amount) {
			break;
		}
	}
	return read;
}

void io_close(usize fd) {
	if (__close_syscall(fd)) {
		io_write(2, str("Failed to close file.\n"));
		die(1);
	}
}

usize io_len(usize fd) {
	struct stat s;
	if (__fstat_syscall(fd, &s)) {
		io_write(2, str("Failed to obtain file length.\n"));
		die(1);
	}
	return s.len;
}
