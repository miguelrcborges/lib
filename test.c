#include "lib.h"

static usize stdErr;
static usize stdOut;
static usize test_count;
static usize test_failed;

#define TEST(test, name) do { \
	test_count++; \
	if (test) {  \
		io_write(stdErr, string("FAILED: ")); \
		io_write(stdErr, string(name"\n")); \
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
	stdOut = getStdOut();
	stdErr = getStdErr();
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

	string_fmtb16(&a, 0, &tmp);
	TEST(!string_compare(string("0"), tmp), "Base16 formatting #1");
	string_fmtb16(&a, -1, &tmp);
	TEST(!string_compare(string("ffffffff"), tmp), "Base16 formatting #2");
	Arena_free(&a);

	string_fmtb8(&a, 0, &tmp);
	TEST(!string_compare(string("0"), tmp), "Base8 formatting #1");
	string_fmtb8(&a, -1, &tmp);
	TEST(!string_compare(string("7777777777777777"), tmp), "Base8 formatting #2");
	Arena_free(&a);

	TEST(!string_compare(string_build(&a, string("Hello,"), string(" world!"), string("\n")), string("Hello, world!\n")), "Variadic String Building");


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
	io_write(stdOut, output);
	return test_failed;
}
