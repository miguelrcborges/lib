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
