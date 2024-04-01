#include "lib.h"

w32(bool) WriteFile(usize fd, const u8 *buffer, u32 len, u32 *written, void *overlapped);
w32(bool) ReadFile(usize fd, const u8 *buffer, u32 len, u32 *written, void *overlapped);
w32(i32) MultiByteToWideChar(u32 code, u64 flags, const u8 *utf8buf, i32 utf8len, u16 *utf16buf, i32 utf16len);
w32(usize) CreateFileW(u16 *fname, u32 flags, u32 shared, void *sec, u64 mode, u64 attributes, int);
w32(i32) CloseHandle(usize fd);
w32(u32) GetFileSize(usize fd, u32 *high);


void io_write(usize fd, string s) {
	usize written = 0;
	while (written < s.len) {
		u32 last_count;
		u32 write_amount = min(s.len - written, UINT32_MAX);
		if (!likely(WriteFile(fd, s.str+written, write_amount, &last_count, NULL))) {
			#define ex(x) (u8*)x, sizeof(x)-1
			u32 written;
			WriteFile(getStdErr(), ex("Failed to write to file.\n"), &written, NULL);
			die(1);
		}
		written += last_count;
	}
}

usize io_read(usize fd, u8 *buf, usize len) {
	usize read = 0;
	u32 last_count;
	while (read < len) {
		u32 read_amount = min(len - read, UINT32_MAX);
		if (!likely(ReadFile(fd, buf+read, read_amount, &last_count, NULL))) {
			io_write(getStdErr(), str("Failed to read file.\n"));
			die(1);
		}
		read += last_count;
		if (last_count < read_amount) {
			break;
		}
	}
	return read;
}

usize io_open(string file, u32 mode) {
	if (unlikely(mode >= IO_MODES_COUNT)) {
		io_write(getStdErr(), str("Invalid mode to open file.\n"));
		die(1);
	}

	u16 conversion_buffer[32768];
	if (MultiByteToWideChar(65001, 0, file.str, file.len, conversion_buffer, 32768) == 0) {
		io_write(getStdErr(), str("Failed to open file.\n"));
		die(1);
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

	usize fd = CreateFileW(conversion_buffer, flags_lookup[mode], 0, 0, mode_lookup[mode], 128, 0);
	if (fd == (usize)-1) {
		io_write(getStdErr(), str("Failed to open file.\n"));
		die(1);
	}
	return fd;
}

void io_close(usize fd) {
	if (CloseHandle(fd) == 0) {
		io_write(getStdErr(), str("Failed to close file.\n"));
		die(1);
	}
}

usize io_len(usize fd) {
	u32 high;
	u32 low = GetFileSize(fd, &high);
	if (high == ((u32) -1)) {
		io_write(getStdErr(), str("Failed to get file size\n"));
		die(1);
	}
	return ((usize)high << 32) | (usize)low;
}
