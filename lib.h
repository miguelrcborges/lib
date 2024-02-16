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

#define len(a) (sizeof(a)/sizeof(*(a))
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
#define clamp(a,x,b) (((x)<(a))?(a):((x)>(b))?(b):(x))

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef size_t usize;
typedef uintptr_t uptr;
typedef ptrdiff_t dptr;
typedef i32 bool;

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
		*(int*)sp._ptr;
	}
	return sp._ptr; 
}

#endif /* LIB_H */
