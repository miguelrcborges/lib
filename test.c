#include "lib.h"

static usize stderr;
static usize stdout;
static usize test_count;
static usize test_failed;

#define TEST(test, name) do { \
	test_count++; \
	if (test) {  \
		io_write(stderr, string("FAILED: ")); \
		io_write(stderr, string(name"\n")); \
		test_failed++; \
	} \
} while (0)


int
#ifdef _WIN32
mainCRTStartup()
#else
main()
#endif
{
	stdout = getStdOut();
	stderr = getStdErr();
	Arena a = Arena_create(0);

	string tmp;
	string_fmti64(&a, 0, &tmp);
	TEST(!string_compare(string("0"), tmp), "Signed number formatting #1");
	string_fmti64(&a, -21, &tmp);
	TEST(!string_compare(string("-21"), tmp), "Signed number formatting #2");
	string_fmti64(&a, 42, &tmp);
	TEST(!string_compare(string("42"), tmp), "Signed number formatting #3");
	string_fmti64(&a, 9223372036854775807, &tmp);
	TEST(!string_compare(string("9223372036854775807"), tmp), "Signed number formatting #4");
	string_fmti64(&a, -9223372036854775808, &tmp);
	TEST(!string_compare(string("-9223372036854775808"), tmp), "Signed number formatting #5");
	Arena_free(&a);

	string_fmtu64(&a, 0, &tmp);
	TEST(!string_compare(string("0"), tmp), "Unsigned number formatting #1");
	string_fmtu64(&a, 42, &tmp);
	TEST(!string_compare(string("42"), tmp), "Unsigned number formatting #2");
	string_fmtu64(&a, 18446744073709551615u, &tmp);
	TEST(!string_compare(string("18446744073709551615"), tmp), "Unsigned number formatting #3");
	Arena_free(&a);


	StringBuilder sb;
	string passed;
	string executed;
	string output;
	string_fmtu64(&a, test_count - test_failed, &passed);
	string_fmtu64(&a, test_count, &executed);
	StringBuilder_create(&sb, &a, passed);
#define a(s) StringBuilder_append(&sb, &a, s);
	a(string("/")); a(executed); a(string(" tests passed.\n"));
#undef a
	StringBuilder_build(&sb, &a, &output);
	io_write(stdout, output);
	return test_failed;
}
