#include "lib.h"

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

w32(void) *VirtualAlloc(void *ptr, usize size, u32 type, u32 protect);
w32(bool) VirtualFree(void *ptr, usize size, u32 type); 
w32(void) ExitProcess(u32 code);
w32(void) GetSystemInfo(SystemInfo *info);

static usize pageSize;

enum CONSTANTS {
	MEM_RESERVE = 0x00002000,
	MEM_COMMIT = 0x00001000,

	MEM_DECOMMIT = 0x00004000,
	MEM_RELEASE = 0x00008000,

	PAGE_READWRITE = 0x04,
};

void *mem_reserve(usize size) {
	void *p = VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_READWRITE); 
	if (unlikely(p == NULL)) {
		io_write(getStdErr(), str("Out of memory.\n"));
		die(1);
	}
	return p;
}

void *mem_rescommit(usize size) {
	void *p = VirtualAlloc(NULL, size, MEM_RESERVE | MEM_RESERVE, PAGE_READWRITE); 
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
	if (!VirtualFree(ptr, size, MEM_RELEASE)) {
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

void die(usize code) {
	ExitProcess(code);
}
