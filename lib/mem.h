#ifndef LIB_MEM_H
#define LIB_MEM_H

#ifndef LIB_H
#include "../lib.h"
#endif

SafePointer mem_reserve(usize size);
SafePointer mem_rescommit(usize size);
bool mem_commit(void *ptr, usize size);
bool mem_decommit(void *ptr, usize size);
bool mem_release(void *ptr, usize size);



typedef struct _ArenaBlock ArenaBlock;
struct _ArenaBlock {
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

Arena Arena_create(FreeList *list);
SafePointer Arena_alloc(Arena *a, usize size, usize alignment);
ArenaState Arena_saveState(Arena *a);
void Arena_rollback(ArenaState a);
void Arena_free(Arena *a);

FreeList FreeList_create(usize block_len);
bool FreeList_release(FreeList *list);

#endif
