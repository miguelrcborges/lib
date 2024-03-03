#include "lib.h"

w32(bool) WriteFile(usize fd, const u8 *buffer, u32 len, u32 *written, void *overlapped);
w32(bool) ReadFile(usize fd, const u8 *buffer, u32 len, u32 *written, void *overlapped);
w32(i32) MultiByteToWideChar(u32 code, u64 flags, const u8 *utf8buf, i32 utf8len, u16 *utf16buf, i32 utf16len);
w32(usize) CreateFileW(u16 *fname, u32 flags, u32 shared, void *sec, u64 mode, u64 attributes, int);
w32(i32) CloseHandle(usize fd);
w32(u32) GetFileSize(usize fd, u32 *high);

bool io_write(usize fd, string s) {
	u32 written;
	return !WriteFile(fd, s.str, s.len, &written, NULL);
}

bool io_read(usize fd, u8 *buf, usize len, usize *written) {
	return !ReadFile(fd, buf, len, (u32 *)written, NULL);
}

bool io_open(string file, u32 mode, usize *fd) {
	if (unlikely(mode >= IO_MODES_COUNT)) {
		return 1;
	}

	u16 conversion_buffer[32768];
	if (MultiByteToWideChar(65001, 0, file.str, file.len, conversion_buffer, 32768) == 0) {
		return 0;
	};

	static u32 flags_lookup[IO_MODES_COUNT] = {
		[IO_READ]   = 1179785,
		[IO_WRITE]  = 1179926, 
		[IO_APPEND] = 1179789,
	};

	static u32 mode_lookup[IO_MODES_COUNT] = {
		[IO_READ]   = 3,
		[IO_WRITE]  = 2, 
		[IO_APPEND] = 4,
	};

	*fd = CreateFileW(conversion_buffer, flags_lookup[mode], 0, 0, mode_lookup[mode], 128, 0);
	return *fd == (usize) -1;
}

bool io_close(usize fd) {
	return CloseHandle(fd) == 0;
}

bool io_readFile(Arena *a, string file, string *content) {
	u16 conversion_buffer[32768];
	if (MultiByteToWideChar(65001, 0, file.str, file.len, conversion_buffer, 32768) == 0) {
		return 0;
	};

	usize fd = CreateFileW(conversion_buffer, 1179785, 0, 0, 3, 128, 0);
	if (unlikely(fd == (usize) -1)) {
		return 1;
	}

	u32 hw;
	u32 lw = GetFileSize(fd, &hw);
	usize len = ((usize)hw << 32) | (usize) lw;
	ArenaState as = Arena_saveState(a);
	SafePointer sp = Arena_alloc(a, len, 1);
	if (unlikely(sp._ptr == NULL)) {
		return 1;
	}
	void *p = sp._ptr;

	// As it is, it can only be read 4 GB on one go.
	if (unlikely(len >= ((usize)1 << 32))) {
		return 1;
	}

	u32 read;
	bool ret = ReadFile(fd, p, len, &read, NULL);

	if (unlikely(ret || read != len)) {
		Arena_rollback(as);
		return 1;
	}

	content->str = sp._ptr;
	content->len = len;
	return 0;
}
