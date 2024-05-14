#include "lib.h"

typedef struct {
	usize num;
	Condition done;
} computeArg;

void computeSum(void *_arg) {
	computeArg *arg = _arg;
	for (usize i = 0; i < 2222; ++i) {
		arg->num += i;
	}
	
	Thread_sleep(100);
	Condition_wake(&(arg->done));
	/* do more stuff like cleanup idk */
	Thread_sleep(4000000);
}

int mainCRTStartup(void) {
	usize stdout = getStdOut();
	Arena a = Arena_create(0);
	computeArg ca = {
		.done = Condition_create(),
	};

	Thread counter = Thread_create(computeSum, &ca);
	io_write(stdout, str("Computing sum.\n"));
	Condition_wait(&(ca.done));
	
	io_write(stdout, string_build(&a, str("Sum: "), string_fmtu64(&a, ca.num), str(".\n")));
	Thread_join(counter);

	return 0;
}
