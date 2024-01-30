#include "lib.h"

w32(bool) WriteFile(usize fd, const char *buffer, u32 len, u32 *written, void *overlapped);
w32(bool) ReadFile(usize fd, const char *buffer, u32 len, u32 *written, void *overlapped);

usize io_open(u8 *file, u32 flags, u32 mode);

bool io_write(usize fd, const char *buf, usize len) {
	u32 written;
	return !WriteFile(fd, buf, len, &written, NULL);
}

bool io_read(usize fd, char *buf, usize len, usize *written) {
	return !ReadFile(fd, buf, len, (u32 *)written, NULL);
}
