#include "lib.h"

usize open_syscall(const u8 *name, i32 flags);

string errInvalidFileOpenMode(void) {
	return str("Tried to open the file with an invalid mode.");
}

string errFailedToOpenFile(void) {
	return str("Failed to open file.");
}

io_open_result io_open(string file, u32 mode) {
	io_open_result ret;
	if (unlikely(mode >= IO_MODES_COUNT)) {
		ret.err.err = errInvalidFileOpenMode;
		return ret;
	}

	static i32 flags_lookup[IO_MODES_COUNT] = {
		[IO_READ]   = 0,
		[IO_WRITE]  = 1001, 
		[IO_APPEND] = 2002,
	};

	if (likely(file.str[file.len] == '\0')) {
		ret.fd = open_syscall(file.str, flags_lookup[mode]);
	} else {
		u8 zeroed[4097];
		usize i = 0;
		for (; i < file.len; i++) {
			zeroed[i] = file.str[i];
		} 
		zeroed[i] = '\0';
		ret.fd = open_syscall(zeroed, flags_lookup[mode]);
	}

	if (ret.fd == (usize) -1) {
		ret.err.err = errFailedToOpenFile;
	}
	return ret;
}
