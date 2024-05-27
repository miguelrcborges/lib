#ifndef LIB_H
#define LIB_H

#include <stdint.h>
#include <stddef.h>

#define STATEMENT(S) do{S}while(0)
#define STRINGIFY_(S) #S
#define STRINGIFY(S) STRINGIFY_(S)
#define GLUE_(A,B) A##B
#define GLUE(A,B) GLUE_(A,B)
#define EXPAND(x) x

#ifndef __has_builtin
	#define __has_builtin(X) (0)
#endif

#ifndef __has_feature
  #define __has_feature(x) (0)
#endif

#ifdef HIDE_ASSERT
	#define assert(x) ((void)0)
#elif defined(_DEBUG)
	#define assert(x) ((x)?(void)0:__failed_assert(str("Failed assert at " __FILE__ ":" STRINGIFY(__LINE__) ": " #x "\n")))
#elif __has_builtin(__builtin_unreachable) 
	#define assert(x) ((x)?(void)0:__builtin_unreachable());
#else
	#define assert(x) ((void)(x))
#endif

#if __has_builtin(__builtin_expect)
	#define likely(expr) __builtin_expect(!!(expr), 1)
	#define unlikely(expr) __builtin_expect(!!(expr), 0)
#else 
	#define likely(expr) (expr)
	#define unlikely(expr) (expr)
#endif

#if __has_feature(c_static_assert)
	#define static_assert(x) _Static_assert(x, #x)
#else
	#define static_assert(x) assert(x)
#endif

#ifdef _WIN32
	#define w32(t) __declspec(dllimport) t __stdcall
#else
	#define w32(t) t
#endif

#if defined(__GNUC__)
	#define force_inline __attribute__((always_inline))
	#define no_inline __attribute__((noinline))
#elif defined(_MSC_VER)
	#define force_inline __forceinline
	#define no_inline __declspec(noinline)
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
#define str(s) (string){(u8*)s, sizeof(s)-1}

typedef struct {
	void *_ptr;
} SafePointer;

typedef struct {
	string (*err)(void);
} GenericError;


#ifndef LIB_H_FREESTANDING

enum IO_OPEN_MODES {
	IO_READ,
	IO_WRITE,
	IO_APPEND,
	IO_MODES_COUNT
};

#if defined(LIBC_BACKEND)
	#include <stdio.h>
	#define getStdIn() ((usize)(stdin))
	#define getStdOut() ((usize)(stdout))
	#define getStdErr() ((usize)(stderr))
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

typedef struct {
	u8 *start;
	u8 *offset;
	u8 *commited;
	u8 *end;
} Arena;

typedef struct {
	Arena *arena;
	u8 *offset;
	u8 *commited;
} ArenaState;

typedef struct _PoolNode PoolNode;
struct _PoolNode {
	PoolNode *next;
};

typedef struct {
	Arena arena;
	usize alloc_size;
	usize alloc_alignment;
	PoolNode *free;
} Pool;


typedef struct stringNode StringNode;
struct stringNode {
	StringNode *next;
	string string;
};

typedef struct {
	StringNode *first;
	StringNode *last;
	usize str_len;
} StringBuilder;


typedef struct {
	void *_state;
} Mutex;

typedef struct {
	void *_state;
	Mutex mux;
} Condition;

typedef struct {
	usize _handle;
} Thread;

/* io.c */
void io_write(usize fd, string s);
usize io_read(usize fd, u8 *buff, usize len);
usize io_open(string file, u32 mode);
usize io_len(usize fd);
void io_close(usize fd);
string io_readFile(Arena *a, string file);


/* mem.c */ 
int __failed_assert(string a);
void die(usize code);
void *mem_reserve(usize size);
void *mem_rescommit(usize size);
void mem_commit(void *ptr, usize size);
void mem_decommit(void *ptr, usize size);
void mem_release(void *ptr, usize size);
usize mem_getPageSize(void);
void mem_copy(void *restrict dest, void *restrict orig, usize len);
Arena Arena_create(usize limit_mem);
void *Arena_alloc(Arena *a, usize size, usize alignment);
ArenaState Arena_saveState(Arena *a);
void Arena_rollback(ArenaState s);
void Arena_free(Arena *a);
Pool Pool_create(usize limit_mem, usize alloc_size, usize alignment);
void *Pool_alloc(Pool *p);
void Pool_free(Pool *p, void *item);
void Pool_clear(Pool *p);


/* str.c */
bool string_equal(string s1, string s2);
i8 string_compare(string s1, string s2);
string string_fmtu64(Arena *a, u64 n);
string string_fmti64(Arena *a, i64 n);
string string_fmtb16(Arena *a, u64 n);
string string_fmtb8(Arena *a, u64 n);
StringBuilder StringBuilder_create(void);
void StringBuilder_append(StringBuilder *sb, Arena *a, string s);
string StringBuilder_build(StringBuilder *sb, Arena *a);
string _string_build(Arena *a, usize n, ...);
#define string_build(a, ...) _string_build(a, sizeof((string[]){__VA_ARGS__})/sizeof(string), __VA_ARGS__)


/* thread.c */
Mutex Mutex_create(void);
bool Mutex_tryLock(Mutex *m);
void Mutex_lock(Mutex *m);
void Mutex_unlock(Mutex *m);
Thread Thread_create(void (*start)(void *data), void *data);
void Thread_join(Thread t);
void Thread_sleep(usize micros);
Condition Condition_create(void);
void Condition_wait(Condition *cond);
void Condition_wake(Condition *cond);
void Condition_broadcast(Condition *cond);


#endif /* LIB_H_FREESTANDING */ 

#endif /* LIB_H */
