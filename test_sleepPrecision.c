#include "src/lib.h"
w32(i32) QueryPerformanceCounter(u64 *perfCount);
w32(i32) QueryPerformanceFrequency(u64 *perfFrequency);

int mainCRTStartup(void) {
	u64 start, end, freq;
	Arena a = Arena_create(0);

	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter (&start);
	Thread_sleep(1000069);
	QueryPerformanceCounter(&end);
	u64 slept_micros = (end - start) * 1000000 / freq;
	io_write(getStdOut(), string_build(&a, str("Slept "), string_fmtu64(&a, slept_micros), str("us (wanted 1000069).\n")));

	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter (&start);
	Thread_sleep(420);
	QueryPerformanceCounter(&end);
	slept_micros = (end - start) * 1000000 / freq;
	io_write(getStdOut(), string_build(&a, str("Slept "), string_fmtu64(&a, slept_micros), str("us (wanted 420).\n")));

	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter (&start);
	Thread_sleep(720027);
	QueryPerformanceCounter(&end);
	slept_micros = (end - start) * 1000000 / freq;
	io_write(getStdOut(), string_build(&a, str("Slept "), string_fmtu64(&a, slept_micros), str("us (wanted 720027).\n")));

	return 0;
}
