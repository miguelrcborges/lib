#include "lib.h"
#include <stdarg.h>

int __failed_assert(string a) {
	io_write(getStdErr(), a);
	die(1);
	return 0;
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
		limit_mem = ((usize)1 << 24);
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

string io_readFile(Arena *a, string file) {
	string r;
	usize fd = io_open(file, IO_READ);
	usize len = io_len(fd);
	r.str = Arena_alloc(a, len, 1); 
	r.len = len;
	io_read(fd, (u8 *)r.str, len);
	return r;
}

bool string_equal(string s1, string s2) {
	if (s1.len != s2.len)
		return 0;

	for (usize i = 0; i < s1.len; ++i)
		if (s1.str[i] != s2.str[i])
			return 0;
	
	return 1;
}

i8 string_compare(string pos, string neg) {
	i8 longer;
	usize limit;
	if (pos.len == neg.len) {
		longer = 0;
		limit = neg.len;
	} else if (pos.len > neg.len) {
		longer = 1;
		limit = neg.len;
	} else {
		longer = -1;
		limit = pos.len;
	}

	for (usize i = 0; i < limit; ++i)
		if (pos.str[i] != neg.str[i])
			// May need to change this later.
			// This wont work properly if values >= 128
			return pos.str[i] - neg.str[i];

	return longer;
}

string string_fmtu64(Arena *a, u64 n) {
	string ret;
	// Max u64 is 18446744073709551615 : 20 characters
	u8 *p = Arena_alloc(a, 20, 1);
	u8 *end = p + 19;
	u8 *cursor = end;
	do {
		*cursor = '0' + n % 10;
		cursor--;
		n /= 10;
	} while (n > 0);
	ret.str = cursor + 1;
	ret.len = end - cursor;
	return ret;
}

string string_fmti64(Arena *a, i64 n) {
	string ret;

	// Widest i64 is -9223372036854775808 : 20 characters
	u8 *p = Arena_alloc(a, 20, 1);
	u64 n2;
	bool is_neg;
	u8 *end = p + 19;
	u8 *cursor = end;
	if (n < 0) {
		is_neg = 1;
		n2 = -n;
	} else {
		is_neg = 0;
		n2 = n;
	}
	
	do {
		*cursor = '0' + n2 % 10;
		cursor--;
		n2 /= 10;
	} while (n2 > 0);
	if (is_neg) {
		*cursor = '-';
		cursor--;
	}
	ret.str = cursor + 1;
	ret.len = end - cursor;
	return ret;
}

StringBuilder StringBuilder_create(void) {
	return (StringBuilder) {0};
};

void StringBuilder_append(StringBuilder *sb, Arena *a, string s) {
	StringNode *n = Arena_alloc(a, sizeof(StringNode), sizeof(void*));
	n->next = NULL;
	n->string = s;
	sb->str_len += s.len;
	if (unlikely(sb->first == NULL)) {
		sb->first = n;
		sb->last = n;
	} else {
		sb->last->next = n;
		sb->last = sb->last->next;
	}
}

string StringBuilder_build(StringBuilder *sb, Arena *a) {
	string ret;
	if (unlikely(sb->str_len == 0)) {
		return str("");
	}

	u8 *cursor = Arena_alloc(a, sb->str_len + 1, 1);
	ret.str = cursor;
	ret.len = sb->str_len;
	for (StringNode *n = sb->first; n; n = n->next) {
		for (usize i = 0; i < n->string.len; ++i) {
			*cursor = n->string.str[i];
			++cursor;
		}
	}
	return ret;
};

string string_fmtb16(Arena *a, u64 n) {
	string ret;
	enum {
		ALLOC_LEN = 64/4,
	};
	static u8 lookup_table[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
	};

	u8 *p = Arena_alloc(a, ALLOC_LEN, 1);
	u8 *end = p + (ALLOC_LEN - 1);
	u8 *cursor = end;
	do {
		*cursor = lookup_table[n&0b1111];
		cursor--;
		n = n >> 4;
	} while (n > 0);
	ret.str = cursor + 1;
	ret.len = end - cursor;
	return ret;
}

string string_fmtb8(Arena *a, u64 n) {
	string ret;
	enum {
		ALLOC_LEN = 64/3,
	};

	u8 *p = Arena_alloc(a, ALLOC_LEN, 1);
	u8 *end = p + (ALLOC_LEN - 1);
	u8 *cursor = end;
	do {
		*cursor = '0' + (n & 0b111);
		cursor--;
		n = n >> 3;
	} while (n > 0);
	ret.str = cursor + 1;
	ret.len = end - cursor;
	return ret;
}

string _string_build(Arena *a, usize n, ...) {
	string ret;

	usize len = 1;
	va_list args;
	va_start(args, n);
	for (usize i = 0; i < n; ++i) {
		len += va_arg(args, string).len;
	}
	va_end(args);

	u8 *writter = Arena_alloc(a, len, 1);
	ret.str = writter; 
	ret.len = len - 1;

	va_start(args, n);
	for (usize i = 0; i < n; ++i) {
		string t = va_arg(args, string);
		for (usize ii = 0; ii < t.len; ++ii) {
			*writter = t.str[ii];
			writter++;
		}
	}
	va_end(args);
	*writter = '\0';

	return ret;
}
