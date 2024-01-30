#include "lib.h"

w32(void) *VirtualAlloc(void *ptr, usize size, u32 type, u32 protect);
w32(bool) VirtualFree(void *ptr, usize size, u32 type); 

enum CONSTANTS {
	MEM_RESERVE = 0x00002000,
	MEM_COMMIT = 0x00001000,

	MEM_DECOMMIT = 0x00004000,
	MEM_RELEASE = 0x00008000,

	PAGE_READWRITE = 0x04,
};

SafePointer mem_reserve(usize size) {
	SafePointer r;
	r._ptr = VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_READWRITE);
	return r;
}

SafePointer mem_rescommit(usize size) {
	SafePointer r;
	r._ptr = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	return r;
}

bool mem_commit(void *ptr, usize size) {
	return (bool) (VirtualAlloc(ptr, size, MEM_COMMIT, 0x04) == NULL);
}

bool mem_decommit(void *ptr, usize size) {
	return VirtualFree(ptr, size, MEM_DECOMMIT);
}

bool mem_release(void *ptr, usize size) {
	return VirtualFree(ptr, size, MEM_DECOMMIT);
}
