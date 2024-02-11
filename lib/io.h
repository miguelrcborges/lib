#ifndef LIB_IO_H
#define LIB_IO_H

#ifndef LIB_H
#include "../lib.h"
#endif

bool io_write(usize fd, string s);
bool io_read(usize fd, u8 *buff, usize len, usize *written);
bool io_open(string file, u32 mode, usize *fd);
bool io_close(usize fd);

enum IO_OPEN_MODES {
	IO_READ,
	IO_WRITE,
	IO_APPEND,
	IO_MODES_COUNT
};

#if defined(LIBC_BACKEND)
 #include "io/libc.h"
#elif defined(_WIN32)
	#include "io/windows.h"
#else
	#include "io/posix.h"
#endif

#endif
