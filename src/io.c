#include "lib.h"

string io_readFile(Arena *a, string file) {
	string r;
	usize fd = io_open(file, IO_READ);
	usize len = io_len(fd);
	r.str = Arena_alloc(a, len, 1); 
	r.len = len;
	io_read(fd, (u8 *)r.str, len);
	return r;
}
