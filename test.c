#include "lib.h"

static usize stdErr;
static usize stdOut;
static usize test_count;
static usize test_failed;

#define TEST(test, name) do { \
	test_count++; \
	if (!(test)) {  \
		io_write(stdErr, str("FAILED: ")); \
		io_write(stdErr, str(name"\n")); \
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

	TEST(string_equal(str("0"), string_fmti64(&a, 0)), "Signed number formatting #1");
	TEST(string_equal(str("-21"), string_fmti64(&a, -21)), "Signed number formatting #2");
	TEST(string_equal(str("42"), string_fmti64(&a, 42)), "Signed number formatting #3");
	TEST(string_equal(str("9223372036854775807"), string_fmti64(&a, 9223372036854775807)), "Signed number formatting #4");
	TEST(string_equal(str("-9223372036854775808"), string_fmti64(&a, -9223372036854775808u)), "Signed number formatting #5");
	Arena_free(&a);

	TEST(string_equal(str("0"), string_fmtu64(&a, 0)), "Unsigned number formatting #1");
	TEST(string_equal(str("42"), string_fmtu64(&a, 42)), "Unsigned number formatting #2");
	TEST(string_equal(str("18446744073709551615"), string_fmtu64(&a, 18446744073709551615u)), "Unsigned number formatting #3");
	Arena_free(&a);

	TEST(string_equal(str("0"), string_fmtb16(&a, 0)), "Base16 formatting #1");
	TEST(string_equal(str("ffffffffffffffff"), string_fmtb16(&a, -1)), "Base16 formatting #2");
	Arena_free(&a);

	TEST(string_equal(str("0"), string_fmtb8(&a, 0)), "Base8 formatting #1");
	TEST(string_equal(str("1777777777777777777777"), string_fmtb8(&a, -1)), "Base8 formatting #2");
	Arena_free(&a);

	TEST(string_equal(string_build(&a, str("Hello,"), str(" world!"), str("\n")), str("Hello, world!\n")), "Variadic String Building");

	StringBuilder sb = StringBuilder_create();
#define a(s) StringBuilder_append(&sb, &a, s);
	a(string_fmtu64(&a, test_count-test_failed)); a(str("/")); a(string_fmtu64(&a, test_count)); 
	a(str(" tests passed.\n"));
#undef a
	io_write(stdOut, StringBuilder_build(&sb, &a));
	return test_failed;
}
