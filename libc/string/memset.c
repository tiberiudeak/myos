// #pragma GCC push_options
// #pragma GCC optimize ("01")

#include <string.h>

void *memset(void *dest, int cons, size_t n) {
	char *d = dest;

	for (size_t i = 0; i < n; i++) {
		d[i] = cons;
	}

	return dest;
}

// #pragma GCC pop_options
