#include <stdlib.h>

void _start(int argc, char *argv[]) {
	int *p = (int *) 0x12345678;
	*p = 1;

	exit(0);
}
