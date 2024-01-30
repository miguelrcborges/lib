#ifndef LIB_IO_H
#define LIB_IO_H

#ifndef LIB_H
#include "../lib.h"
#endif

usize io_open(string file, u32 flags, u32 mode);
bool io_write(usize fd, const char *buf, usize len);
bool io_read(usize fd, char *buff, usize len, usize *written);

#ifdef _WIN32
	#include "io/windows.h"
#else
	#define getStdIn() 0
	#define getStdOut() 1
	#define getStdErr() 2
#endif

#endif
