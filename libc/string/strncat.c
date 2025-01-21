#include <string.h>

char *strncat(char *dest, const char *src, size_t n) {
	size_t index = strlen(dest);
	char *temp = dest;

	for (size_t i = 0; i < n && src[i] != '\0'; i++) {
		temp[index + i] = src[i];
	}

	temp[index] = '\0';
	return dest;
}
