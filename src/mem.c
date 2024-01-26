#include "lib/mem.h"

#if !__has_builtin(__asan_poison_memory_region)
	#define __asan_poison_memory_region(addr, length) ((void) 0)
#endif

#if !__has_builtin(__asan_unpoison_memory_region)
	#define __asan_unpoison_memory_region(addr, length) ((void) 0)
#endif

static FreeList defaultFreeList = {
	.first = NULL,
	.last = NULL,
	.block_len = 4096
};

Arena Arena_create(FreeList *list) {
	if (list == NULL) {
		list = &defaultFreeList;
	}
	return (Arena) {
		.first = NULL,
		.current = NULL,
		.free = list
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
		if (unlikely(a->free->first == NULL || a->free->block_len > maxoff)) {
			SafePointer p = mem_rescommit(maxoff);
			if (p._ptr == NULL) {
				return p;
			}
			a->current = p._ptr;
			a->current->offset = (void *)a->current + sizeof(ArenaBlock);
			a->current->end = (void *)a->current + maxoff;
			__asan_poison_memory_region(a->current->offset, maxoff);
		} else {
			a->current = a->free->first;
			a->free->first = a->free->first->next;
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

ArenaState Arena_saveState(Arena *a) {
	return (ArenaState) {
		.arena = a,
		.current = a->current,
		.offset = a->current->offset
	};
}

void Arena_rollback(ArenaState s) {
	s.arena->current = s.current;
	s.arena->current->offset = s.offset;
	dptr free = s.arena->current->end - s.arena->current->offset;
	__asan_poison_memory_region(a->arena->current->offset, free); 
	for (ArenaBlock *p = s.current->next; p != NULL; p = p->next) {
		u8 *base = (u8 *)p + sizeof(ArenaBlock);
		dptr free = p->end - base;
		p->offset = base;
		__asan_poison_memory_region(base, free); 
	}
	if (s.arena->free->first == NULL) {
		s.arena->free->first = s.arena->current->next;
		s.arena->free->last = s.arena->free->first;
	}
	s.arena->free->last->next = s.arena->current->next;
	s.arena->current->next = NULL;
}

void Arena_free(Arena *a) {
	for (ArenaBlock *p = a->current; p != NULL; p = p->next) {
		u8 *base = (u8 *)p + sizeof(ArenaBlock);
		dptr free = p->end - base;
		p->offset = base;
		__asan_poison_memory_region(base, free); 
	}
	if (a->free->first == NULL) {
		a->free->first = a->current;
		a->free->last = a->free->first;
	}
	a->free->last->next = a->current;
	a->first = NULL;
	a->current= NULL;
}

FreeList FreeList_create(usize block_len) {
	return (FreeList) {
		.first = NULL,
		.last = NULL,
		.block_len = block_len
	};
}

bool FreeList_release(FreeList *list) {
	bool r = 0;
	while (list->first != NULL) {
		ArenaBlock *c = list->first;
		list->first = c->next;
		if (unlikely(r = mem_release(c, c->end - (u8 *)c))) {
			list->first = c;
			return r;
		}
	}
	list->last = NULL;
	return 0;
}
