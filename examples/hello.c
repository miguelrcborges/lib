#include "lib.h"

int
#ifdef _WIN32
mainCRTStartup()
#else
main()
#endif
{
	io_write(getStdOut(), str("Hello, world!\n"));
	return 0;
}
