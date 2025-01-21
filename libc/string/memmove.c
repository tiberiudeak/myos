#include <string.h>

void *memmove(void *dest, const void *src, size_t n) {
	char *d = dest;
	const char *s = src;

	if (d < s) {
		while (n--) {
			*d++ = *s++;
		}
	} else {
		size_t i = 0;

		while (i < n) {
			d[n - i - 1] = s[n - i - 1];
			i++;
		}
	}

	return dest;
}
