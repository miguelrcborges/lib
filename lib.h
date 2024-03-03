#ifndef LIB_H
#define LIB_H

#include <stdint.h>
#include <stddef.h>

#define STATEMENT(S) do{S}while(0)
#define STRINGIFY_(S) #S
#define STRINGIFY(S) STRINGIFY_(S)
#define GLUE_(A,B) A##B
#define GLUE(A,B) GLUE_(A,B)

#ifndef __has_builtin
	#define __has_builtin(X) (0)
#endif

#ifndef __has_feature
  #define __has_feature(x) (0)
#endif

#ifdef HIDE_ASSERT
	#define assert(x) ((void)0)
#elif __has_builtin(__builtin_unreachable)
	#define assert(x) ((x)?(void)0:__builtin_unreachable());
#elif defined(_DEBUG)
	#define assert(x) ((x)?(void)0:*(int*)0)
#else
	#define assert(x) ((void)0)
#endif

#if __has_builtin(__builtin_expect)
	#define likely(expr) __builtin_expect(!!(expr), 1)
	#define unlikely(expr) __builtin_expect(!!(expr), 0)
#else 
	#define likely(expr) (expr)
	#define unlikely(expr) (expr)
#endif

#if __has_feature(c_static_assert)
	#define static_assert(x, message) _Static_assert(x, message)
#else
	#define static_assert(x, message) assert(x)
#endif

#ifdef _WIN32
	#define w32(t) __declspec(dllimport) t __stdcall
#else
	#define w32(t) t
#endif

#if defined(__GNUC__)
    #define force_inline __attribute__((always_inline))
#elif defined(_MSC_VER)
    #define force_inline __forceinline
#else
    #define force_inline inline
#endif

#define len(a) (sizeof(a)/sizeof(*(a)))
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
#define clamp(a,x,b) (((x)<(a))?(a):((x)>(b))?(b):(x))

typedef int_least8_t i8;
typedef int_least16_t i16;
typedef int_least32_t i32;
typedef int_least64_t i64;

typedef uint_least8_t u8;
typedef uint_least16_t u16;
typedef uint_least32_t u32;
typedef uint_least64_t u64;

typedef float f32;
typedef double f64;

typedef size_t usize;
typedef uintptr_t uptr;
typedef ptrdiff_t dptr;
typedef u8 bool;

typedef struct {
	const u8 *str;
	usize len;
} string;
#define string(s) (string){(u8*)s, sizeof(s)-1}

typedef struct {
	void *_ptr;
} SafePointer;

static force_inline void *unwrap(SafePointer sp) {
	if (unlikely(sp._ptr == NULL)) {
		#if __has_builtin(__builtin_trap)
			__builtin_trap();
		#else
			(void)*(int*)0;
		#endif
	}
	return sp._ptr; 
}

#ifndef LIB_H_FREESTANDING

enum IO_OPEN_MODES {
	IO_READ,
	IO_WRITE,
	IO_APPEND,
	IO_MODES_COUNT
};

#if defined(LIBC_BACKEND)
	#include <stdio.h>
	#define getStdIn() ((usize)stdin)
	#define getStdOut() ((usize)stdout)
	#define getStdErr() ((usize)stderr)
#elif defined(_WIN32)
	w32(u32) GetStdHandle(u32 nhandle);
	#define getStdIn() (GetStdHandle((u32)-10))
	#define getStdOut() (GetStdHandle((u32)-11))
	#define getStdErr() (GetStdHandle((u32)-12))
#else
	#define getStdIn() 0
	#define getStdOut() 1
	#define getStdErr() 2
#endif

typedef struct arenaBlock ArenaBlock;
struct arenaBlock {
	u8 *offset;
	u8 *end;
	ArenaBlock *next;
};

typedef struct {
	ArenaBlock *first;
	ArenaBlock *last;
	usize block_len;
} FreeList;

typedef struct {
	ArenaBlock *first;
	ArenaBlock *current;
	FreeList *free;
} Arena;

typedef struct {
	Arena *arena;
	ArenaBlock *current;
	u8 *offset;
} ArenaState;

typedef struct poolFreeList PoolFreeList;
struct poolFreeList {
	PoolFreeList *next;
};

typedef struct {
	Arena arena;
	PoolFreeList *free;
	usize alloc_size;
	usize alloc_alignment;
} Pool;

typedef struct {
	void *_state;
} Mutex;


/* io.c */
bool io_write(usize fd, string s);
bool io_read(usize fd, u8 *buff, usize len, usize *written);
bool io_open(string file, u32 mode, usize *fd);
bool io_close(usize fd);
bool io_readFile(Arena *a, string file, string *content);


/* mem.c */ 
SafePointer mem_reserve(usize size);
SafePointer mem_rescommit(usize size);
bool mem_commit(void *ptr, usize size);
bool mem_decommit(void *ptr, usize size);
bool mem_release(void *ptr, usize size);
Arena Arena_create(FreeList *list);
SafePointer Arena_alloc(Arena *a, usize size, usize alignment);
ArenaState Arena_saveState(Arena *a);
void Arena_rollback(ArenaState a);
void Arena_free(Arena *a);
FreeList FreeList_create(usize block_len);
bool FreeList_release(FreeList *list);
Pool Pool_create(FreeList *list, usize alloc_size, usize alignment);
SafePointer Pool_alloc(Pool *p);
void Pool_free(Pool *p, void *item);
void Pool_clear(Pool *p);


/* thread.c */
Mutex Mutex_create(void);
bool Mutex_tryLock(Mutex *m);
void Mutex_lock(Mutex *m);
void Mutex_unlock(Mutex *m);

#endif /* LIB_H_FREESTANDING */ 

#endif /* LIB_H */
