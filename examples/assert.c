#include "lib.h"

int
#ifdef _WIN32
mainCRTStartup()
#else
main()
#endif
{
	assert(5 == 2);
	return 0;
}
