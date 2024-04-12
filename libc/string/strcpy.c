#include <string.h>

char *strcpy(char *dest, const char *src) {
	char *temp = dest;

    char *test = "ana are mere";

	while (*src != '\0') {
		*temp = *src;
		temp++;
		src++;
	}

	*temp = '\0';
	return dest;
}
