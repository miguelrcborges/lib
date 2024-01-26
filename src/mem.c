#include "lib/mem.h"

#if !__has_builtin(__asan_poison_memory_region)
	#define __asan_poison_memory_region(addr, length) ((void) 0)
#endif

#if !__has_builtin(__asan_unpoison_memory_region)
	#define __asan_unpoison_memory_region(addr, length) ((void) 0)
#endif

static FreeList defaultFreeList = {
	.list = NULL,
	.block_len = 4096,
};

Arena Arena_create() {
	return (Arena) {
		.first = NULL,
		.current = NULL,
		.free = &defaultFreeList,
	};
};

Arena Arena_createCustom(FreeList *list) {
	return (Arena) {
		.first = NULL,
		.current = NULL,
		.free = list,
	};
};

SafePointer Arena_alloc(Arena *a, usize size, usize alignment) {
	if (likely(a->current != NULL)) {
		usize left_pad = alignment - ((uptr) a->current->offset & (alignment - 1));
		if (unlikely(a->current->end - a->current->offset < size + left_pad))
			a->current = NULL;
	}
	if (unlikely(a->current == NULL)) {
		usize maxoff = size + alignment + sizeof(ArenaBlock);
		if (unlikely(a->free->list == NULL || a->free->block_len > maxoff)) {
			SafePointer p = mem_rescommit(maxoff);
			if (p._ptr == NULL) {
				return p;
			}
			a->current = p._ptr;
			a->current->offset = (void *)a->current + sizeof(ArenaBlock);
			a->current->end = (void *)a->current + maxoff;
			__asan_poison_memory_region(a->current->offset, maxoff);
		} else {
			a->current = a->free->list;
			a->free->list = a->free->list->next;
		}
		if (unlikely(a->first == NULL)) {
			a->first = a->current;
		}
	} 

	usize left_pad = alignment - ((uptr) a->current->offset & (alignment - 1));
	SafePointer r;
	a->current->offset += left_pad;
	r._ptr = a->current;
	__asan_unpoison_memory_region(a->current->offset, size);
	a->current += size;
	return r;
}

ArenaState Arena_saveState(Arena *a);
void Arena_rollback(ArenaState a);
void Arena_free(Arena *a);
