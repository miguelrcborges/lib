#include "lib.h"

#if defined(__SANITIZE_ADDRESS__) || __has_feature(address_sanitizer)
  void __asan_poison_memory_region(void* addr, size_t size);
  void __asan_unpoison_memory_region(void* addr, size_t size);
  void* __asan_region_is_poisoned(void* addr, size_t size);
#else
  #define __asan_poison_memory_region(addr, size)
  #define __asan_unpoison_memory_region(addr, size)
  #define __asan_region_is_poisoned(addr, size) 0
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
			if (a->free->first == NULL || a->free->block_len < maxoff) {
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
				a->free->first = a->free->first->next;
				a->current->next = 0;
			}
		}
	} else {
		if (unlikely(a->free->first == NULL || a->free->block_len < maxoff)) {
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
			a->first = a->current;
			a->free->first = a->free->first->next;
			a->current->next = 0;
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
		.offset = a->current ? a->current->offset : NULL
	};
}

void Arena_rollback(ArenaState s) {
	if (unlikely(s.current == NULL))
		return Arena_free(s.arena);
		
	s.arena->current = s.current;
	s.arena->current->offset = s.offset;
	dptr free = s.arena->current->end - s.arena->current->offset;
	__asan_poison_memory_region(s.arena->current->offset, free); 
	if (s.current->next) {
		for (ArenaBlock *p = s.current->next; p != NULL; p = p->next) {
			u8 *base = (u8 *)p + sizeof(ArenaBlock);
			dptr free = p->end - base;
			p->offset = base;
			__asan_poison_memory_region(base, free); 
		}
		if (s.arena->free->first == NULL) {
			s.arena->free->first = s.arena->current->next;
			s.arena->free->last = s.arena->free->first;
		} else {
			s.current->next->next = s.arena->free->first;
			s.arena->free->first = s.current->next;
			s.arena->current->next = 0;
		}
	}	
}

void Arena_free(Arena *a) {
	if (unlikely(a->current == NULL)) {
		return;
	}
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
		a->current->next = a->free->first;
		a->free->first = a->first;
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

Pool Pool_create(FreeList *list, usize alloc_size, usize alignment) {
	return (Pool) {
		.arena = Arena_create(list),
		.free = NULL,
		.alloc_size = alloc_size,
		.alloc_alignment = alignment,
	};
}

SafePointer Pool_alloc(Pool *p) {
	if (p->free != NULL) {
		SafePointer r;
		r._ptr = (void *)p->free;
		p->free = p->free->next;
		return r;
	}
	return Arena_alloc(&(p->arena), p->alloc_size, p->alloc_alignment);
}

void Pool_free(Pool *p, void *item) {
	PoolFreeList *new = item;
	new->next = p->free;
	p->free = new;
}

void Pool_clear(Pool *p) {
	Arena_free(&(p->arena));
	p->free = NULL;
}

void mem_copy(void *restrict dest, void *restrict orig, usize len) {
	u8 *d = dest;
	u8 *o = orig;
	if (d > o) {
		do {
			--len;
			d[len] = o[len];
		} while (len);
	} else {
		for (usize i = 0; i < len; ++i) {
			d[i] = o[i];
		}
	}
}
