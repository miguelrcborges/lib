#include "lib.h"

w32(bool) TryAcquireSRWLockExclusive(Mutex *m);
w32(void) AcquireSRWLockExclusive(Mutex *m);
w32(void) ReleaseSRWLockExclusive(Mutex *m);
w32(usize) CreateThread(void *attr, usize stackSize, void (*fun)(void*), void *data, u32 flags, u32 *threadId);
w32(u32) WaitForSingleObject(usize handle, u32 milis);
w32(i32) QueryPerformanceCounter(u64 *perfCount);
w32(i32) QueryPerformanceFrequency(u64 *perfFrequency);
w32(void) Sleep(u32 milis);
w32(long) NtSetTimerResolution(unsigned long resolution, i32 bSet, unsigned long *currentRes);

static u64 queryPerformanceFrequency;

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

Thread Thread_create(void (*start)(void *data), void *data) {
	Thread t;
	t._handle = CreateThread(
		NULL,
		0,
		start,
		data,
		0,
		NULL
	);
	if (unlikely(t._handle == 0)) {
		io_write(getStdErr(), str("Failed to create thread.\n"));
		die(1);
	}
	return t;
}

void Thread_join(Thread t) {
	if (unlikely(WaitForSingleObject(t._handle, 0xffffffff) == 0xfffffff)) {
		io_write(getStdErr(), str("Failed to join thread.\n"));
		die(1);
	}
}

// http://www.geisswerks.com/ryan/FAQS/timing.html
void Thread_sleep(u64 micros) {
	u64 start_time;
	QueryPerformanceCounter(&start_time);
	if (unlikely(queryPerformanceFrequency == 0)) {
		unsigned long tmp;
		QueryPerformanceFrequency(&queryPerformanceFrequency);
		NtSetTimerResolution(5000, 1, &tmp);
	}
	u64 end = start_time + (micros * queryPerformanceFrequency) / 1000000;
	i64 sleep = micros - 2000;
	if (sleep > 0) {
		Sleep(sleep / 1000);
	}
	for (;;) {
		u64 current;
		QueryPerformanceCounter(&current);
		if (current > end) {
			break;
		} else {
			Sleep(0); Sleep(0); Sleep(0);
			Sleep(0); Sleep(0); Sleep(0);
		}
	}
}
