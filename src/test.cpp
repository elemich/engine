#include <stdio.h>

int main()
{
	#ifdef __CYGWIN__
	printf("this is a CYGWIN program...\n");
	#else
	printf("this is not a CYGWIN program...\n");
	#endif
	return 1;
}