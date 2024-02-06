#include <string.h>

char * strncpy(char *dest, const char *src, size_t n) {
	char *temp = dest;
	size_t i;

	for (i = 0; i < n && src[i] != '\0'; i++) {
		temp[i] = src[i];
	}

	for (; i < n; i++) {
		temp[i] = '\0';
	}

	return dest;
}
