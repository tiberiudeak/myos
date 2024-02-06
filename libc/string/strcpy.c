#include <string.h>

char *strcpy(char *dest, const char *src) {
	char *temp = dest;

	while (*src != '\0') {
		*temp = *src;
		temp++;
		src++;
	}

	*temp = '\0';
	return dest;
}
