#ifndef LIB_THREAD_H
#define LIB_THREAD_H

#ifndef LIB_H
#include "../lib.h"
#endif

typedef struct {
	void *_state;
} Mutex;

Mutex Mutex_create(void);
bool Mutex_tryLock(Mutex *m);
void Mutex_lock(Mutex *m);
void Mutex_unlock(Mutex *m);

#endif
