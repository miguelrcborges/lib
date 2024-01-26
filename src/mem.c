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
	usize maxoff = size + alignment + sizeof(ArenaBlock);
	if (likely(a->current != NULL)) {
		usize left_pad = (alignment - ((uptr) a->current->offset & (alignment - 1))) & (alignment - 1);
		if (unlikely(a->current->end - a->current->offset < size + left_pad)) {
			if (a->free->first == NULL || a->free->block_len > maxoff) {
				usize asize = max(maxoff, a->free->block_len);
				SafePointer p = mem_rescommit(asize);
				if (p._ptr == NULL) {
					return p;
				}
				a->current->next = p._ptr;
				a->current = a->current->next;
				a->current->offset = (void *)a->current + sizeof(ArenaBlock);
				a->current->end = (void *)a->current + asize;
				__asan_poison_memory_region(a->current->offset, asize - sizeof(ArenaBlock));
			} else {
				a->current->next = a->free->first;
				a->current = a->current->next;
			}
		}
	} else {
		if (a->free->first == NULL || a->free->block_len > maxoff) {
			usize asize = max(maxoff, a->free->block_len);
			SafePointer p = mem_rescommit(asize);
			if (p._ptr == NULL) {
				return p;
			}
			a->first = p._ptr;
			a->current = p._ptr;
			a->current->offset = (u8 *)a->current + sizeof(ArenaBlock);
			a->current->end = (u8 *)a->current + asize;
			__asan_poison_memory_region(a->current->offset, asize - sizeof(ArenaBlock));
		} else {
			a->current = a->free->first;
			a->free->first = a->free->first->next;
		}
	} 
	usize left_pad = (alignment - ((uptr) a->current->offset & (alignment - 1))) & (alignment - 1);
	SafePointer r;
	a->current->offset += left_pad;
	r._ptr = a->current->offset;
	__asan_unpoison_memory_region(a->current->offset, size);
	a->current->offset += size;
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
	for (ArenaBlock *p = a->first; p != NULL; p = p->next) {
		u8 *base = (u8 *)p + sizeof(ArenaBlock);
		dptr free = p->end - base;
		p->offset = base;
		__asan_poison_memory_region(base, free); 
	}
	if (a->free->first == NULL) {
		a->free->first = a->current;
		a->free->last = a->free->first;
	} else {
		a->free->last->next = a->current;
	}
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
	if (list == NULL) {
		list = &defaultFreeList;
	}
	while (list->first != NULL) {
		ArenaBlock *c = list->first;
		list->first = c->next;
		if (unlikely(r = mem_release(c, c->end - (u8 *)c))) {
			list->first = c;
			return r;
		}
	}
	list->last = NULL;
	return r;
}
