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


Arena Arena_create(usize limit_mem) {
	Arena r;
	if (limit_mem == 0) {
		limit_mem = ((usize)1 << 32);
	}

	r.start = mem_reserve(limit_mem);
	r.end = r.start + limit_mem;
	r.offset = r.start;
	r.commited = r.start;
	__asan_poison_memory_region(r.start, (usize) (r.end-r.start)); 
	return r;
}

void *Arena_alloc(Arena *a, usize size, usize alignment) {
	u8 *p = a->offset;
	assert((alignment&(alignment-1)) == 0);
	p += ((usize)(-(usize)p)) & (alignment - 1);
	u8 *alloc_end = p + size;
	if (alloc_end >= a->commited) {
		if (unlikely(alloc_end >= a->end)) {
			io_write(getStdErr(), str("Arena went out of virtual memory.\n"));
			die(1);
		}
		usize pageSize = mem_getPageSize();
		assert((pageSize&(pageSize-1)) == 0);
		u8 *commit_end = alloc_end;
		commit_end += (usize) (((uptr)-(uptr)alloc_end) & (pageSize-1));
		mem_commit(a->commited, commit_end - a->commited);
		a->commited = commit_end;
	}
	__asan_unpoison_memory_region(p, size); 
	a->offset = alloc_end;
	return p;
}

ArenaState Arena_saveState(Arena *a) {
	return (ArenaState) {
		.arena = a,
		.commited = a->commited,
		.offset = a->offset,
	};
}

void Arena_rollback(ArenaState s) {
	if (likely(s.commited != s.arena->commited)) {
		mem_decommit(s.commited, (usize) (s.commited - s.arena->commited));
	}
	__asan_poison_memory_region(s.offset, (usize) (s.arena->offset - s.offset)); 
	s.arena->offset = s.offset;
}

void Arena_free(Arena *a) {
	if (likely(a->commited != a->start)) {
		mem_decommit(a->start, (usize) (a->commited - a->start));
		a->commited = a->start;
	}
	a->offset = a->start;
	__asan_poison_memory_region(r.start, (usize) (r.end-r.start)); 
}

Pool Pool_create(usize limit_mem, usize alloc_size, usize alignment) {
	if (unlikely(alloc_size < sizeof(void*)))
		alloc_size = 8;
	return (Pool) {
		.arena = Arena_create(limit_mem),
		.alloc_size = alloc_size,
		.alloc_alignment = alignment,
	};
}

void *Pool_alloc(Pool *p) {
	if (p->free != NULL) {
		void *r = p->free;
		__asan_unpoison_memory_region(r, p->alloc_size); 
		p->free = p->free->next;
		return r;
	}
	return Arena_alloc(&(p->arena), p->alloc_size, p->alloc_alignment);
}

void Pool_free(Pool *p, void *item) {
	PoolNode *new = item;
	new->next = p->free;
	p->free = new;
	__asan_poison_memory_region(item, p->alloc_size); 
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

int __failed_assert(string a) {
	io_write(getStdErr(), a);
	die(1);
	return 0;
}
