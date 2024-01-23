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
typedef ssize_t isize;
typedef uintptr_t uptr;
typedef ptrdiff_t dptr;

typedef struct {
	u8 *str;
	size_t len;
} s8;
#define s8(s) (s8){(u8*)s, sizeof(s)-1}

typedef struct {
	u16 *str;
	size_t len;
} s16;
#define s16(s) (s16){(u16*)u##s, len((u##s))-1}


typedef struct SafePointer SafePointer;


#ifdef HIDE_ASSERT
	#define assert(x) ((void)0)
#elif __has_builtin(__builtin_unreachable)
	#define assert(x) ((x)?(void)0:__builtin_unreachable());
#elif defined(_DEBUG)
	#define assert(x) ((x)?(void)0:*(int*)0)
#else
	#define assert(x) ((void)0)
#endif

#endif /* LIB_H */
