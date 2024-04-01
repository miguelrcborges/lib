#include "lib.h"

int
#ifdef _WIN32
mainCRTStartup()
#else
main()
#endif
{
	usize f = io_open(str("test.txt"), IO_WRITE);
	io_write(f, str("it works\n"));
	return 0;
}
