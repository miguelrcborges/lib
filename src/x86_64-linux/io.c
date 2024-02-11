#include "lib/io.h"

i64 open_syscall(const u8 *name, i32 flags);

bool io_open(string file, u32 mode, usize *fd) {
	static i32 flags_lookup[IO_MODES_COUNT] = {
		[IO_READ]   = 0,
		[IO_WRITE]  = 1001, 
		[IO_APPEND] = 2002,
	};

	int t_fd;
	if (likely(file.str[file.len] == '\0')) {
		t_fd = open_syscall(file.str, flags_lookup[mode]);
	} else {
		u8 zeroed[4097];
		usize i = 0;
		for (; i < file.len; i++) {
			zeroed[i] = file.str[i];
		} 
		zeroed[i] = '\0';
		t_fd = open_syscall(zeroed, flags_lookup[mode]);
	}

	*fd = (usize) t_fd;
	return t_fd == -1;
}
