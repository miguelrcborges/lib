#ifndef LIB_MEM_H
#define LIB_MEM_H

#ifndef LIB_H
#include "../lib.h"
#endif

SafePointer mem_reserve(usize size);
bool mem_commit(void *ptr, usize size);
bool mem_decommit(void *ptr, usize size);
bool mem_release(void *ptr, usize size);

#endif
