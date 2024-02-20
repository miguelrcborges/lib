#include "lib/thread.h"

w32(bool) TryAcquireSRWLockExclusive(Mutex *m);
w32(void) AcquireSRWLockExclusive(Mutex *m);
w32(void) ReleaseSRWLockExclusive(Mutex *m);

Mutex Mutex_create(void) {
	return (Mutex) {
		._state = 0,
	};
}

bool Mutex_tryLock(Mutex *m) {
	return TryAcquireSRWLockExclusive(m);
}

void Mutex_lock(Mutex *m) {
	AcquireSRWLockExclusive(m);
}

void Mutex_unlock(Mutex *m) {
	ReleaseSRWLockExclusive(m);
}
