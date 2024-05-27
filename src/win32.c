#include "lib.h"

#include "compilercope.c"
#include "common.c"

typedef struct {
	union {
		u32 OemId;
		struct {
			u16 arch;
			u16 res;
		} dummy;
	} dummyunion;
	u32 pageSize;
	void *minAddress;
	void *maxAddress;
	u32 processorMask;
	u32 numberOfCPUs;
	u32 processorType;
	u32 allocGranularity;
	u16 processorLevel;
	u16 processorRevision;
} SystemInfo;

w32(void *) VirtualAlloc(void *ptr, usize size, u32 type, u32 protect);
w32(bool) VirtualFree(void *ptr, usize size, u32 type); 
w32(void) ExitProcess(u32 code);
w32(void) GetSystemInfo(SystemInfo *info);
w32(bool) WriteFile(usize fd, const u8 *buffer, u32 len, u32 *written, void *overlapped);
w32(bool) ReadFile(usize fd, const u8 *buffer, u32 len, u32 *written, void *overlapped);
w32(i32) MultiByteToWideChar(u32 code, u64 flags, const u8 *utf8buf, i32 utf8len, u16 *utf16buf, i32 utf16len);
w32(usize) CreateFileW(u16 *fname, u32 flags, u32 shared, void *sec, u64 mode, u64 attributes, int);
w32(i32) CloseHandle(usize fd);
w32(u32) GetFileSize(usize fd, u32 *high);
w32(bool) TryAcquireSRWLockExclusive(Mutex *m);
w32(void) AcquireSRWLockExclusive(Mutex *m);
w32(void) ReleaseSRWLockExclusive(Mutex *m);
w32(usize) CreateThread(void *attr, usize stackSize, void (*fun)(void*), void *data, u32 flags, u32 *threadId);
w32(u32) WaitForSingleObject(usize handle, u32 milis);
w32(i32) QueryPerformanceCounter(u64 *perfCount);
w32(i32) QueryPerformanceFrequency(u64 *perfFrequency);
w32(void) Sleep(u32 milis);
w32(long) NtSetTimerResolution(unsigned long resolution, i32 bSet, unsigned long *currentRes);


static usize pageSize;

enum CONSTANTS {
	MEM_RESERVE = 0x00002000,
	MEM_COMMIT = 0x00001000,

	MEM_DECOMMIT = 0x00004000,
	MEM_RELEASE = 0x00008000,

	PAGE_READWRITE = 0x04,
};

void die(usize code) {
	ExitProcess(code);
}

void *mem_reserve(usize size) {
	void *p = VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_READWRITE); 
	if (unlikely(p == NULL)) {
		io_write(getStdErr(), str("Out of memory.\n"));
		die(1);
	}
	return p;
}

void *mem_rescommit(usize size) {
	void *p = VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE); 
	if (unlikely(p == NULL)) {
		io_write(getStdErr(), str("Out of memory.\n"));
		die(1);
	}
	return p;
}

void mem_commit(void *ptr, usize size) {
	void *p = VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
	if (unlikely(p == NULL)) {
		io_write(getStdErr(), str("Out of memory.\n"));
		die(1);
	}
	return;
}

void mem_decommit(void *ptr, usize size) {
	if (!VirtualFree(ptr, size, MEM_DECOMMIT)) {
		io_write(getStdErr(), str("Failed to decommit memory.\n"));
		die(1);
	}
	return;
}

void mem_release(void *ptr, usize size) {
	(void)(size);
	if (!VirtualFree(ptr, 0, MEM_RELEASE)) {
		io_write(getStdErr(), str("Failed to release memory.\n"));
		die(1);
	}
	return;
}

usize mem_getPageSize(void) {
	if (likely(pageSize))
		return pageSize;

	SystemInfo si;
	GetSystemInfo(&si);
	pageSize = si.pageSize;
	return pageSize;
}


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


static u64 queryPerformanceFrequency;

Mutex Mutex_create(void) {
	return (Mutex) {
		._state = 0,
	};
}

bool Mutex_tryLock(Mutex *m) {
	return TryAcquireSRWLockExclusive(m);
}

void Mutex_lock(Mutex *m) {
	AcquireSRWLockExclusive(m);
}

void Mutex_unlock(Mutex *m) {
	ReleaseSRWLockExclusive(m);
}

Thread Thread_create(void (*start)(void *data), void *data) {
	Thread t;
	t._handle = CreateThread(
		NULL,
		0,
		start,
		data,
		0,
		NULL
	);
	if (unlikely(t._handle == 0)) {
		io_write(getStdErr(), str("Failed to create thread.\n"));
		die(1);
	}
	return t;
}

void Thread_join(Thread t) {
	if (unlikely(WaitForSingleObject(t._handle, 0xffffffff) == 0xfffffff)) {
		io_write(getStdErr(), str("Failed to join thread.\n"));
		die(1);
	}
}

// http://www.geisswerks.com/ryan/FAQS/timing.html
void Thread_sleep(u64 micros) {
	u64 start_time;
	QueryPerformanceCounter(&start_time);
	if (unlikely(queryPerformanceFrequency == 0)) {
		unsigned long tmp;
		QueryPerformanceFrequency(&queryPerformanceFrequency);
		NtSetTimerResolution(5000, 1, &tmp);
	}
	u64 end = start_time + (micros * queryPerformanceFrequency) / 1000000;
	i64 sleep = micros - 2000;
	if (sleep > 0) {
		Sleep(sleep / 1000);
	}
	for (;;) {
		u64 current;
		QueryPerformanceCounter(&current);
		if (current > end) {
			break;
		} else {
			Sleep(0); Sleep(0); Sleep(0);
		}
	}
}


// https://github.com/skeeto/w64devkit/blob/master/src/libchkstk.S
#ifdef __GNUC__
	#ifdef __amd64
no_inline void ___chkstk_ms(void) {
	asm volatile (
		"push %rax\n\t"
		"push %rcx\n\t"
		"mov %gs:(0x10), %rcx\n\t"
		"neg %rax\n\t"
		"add %rsp, %rax\n\t"
		"jb 1f\n\t"
		"xor %eax, %eax\n"
		"0:\tsub $0x1000, %rcx\n\t"
		"test %eax, (%rcx)\n"
		"1:\tcmp %rax, %rcx\n\t"
		"ja 0b\n\t"
		"pop %rcx\n\t"
		"pop %rax\n\t"
	);
}
	#else
	#endif
#else
	#ifdef _WIN64
//void ___chkstk_ms() {
	#else
//void __chkstk() {
	#endif
#endif


