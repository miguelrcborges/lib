#include "lib.h"

int
#ifdef _WIN32
mainCRTStartup()
#else
main()
#endif
{
	Arena a = Arena_create(0);
	string content = io_readFile(&a, str("readFile.c"));
	io_write(getStdOut(), content);
	return 0;
}
