#include <string.h>

char *strcat(char *dest, const char *src) {
	size_t index = strlen(dest);
	char *temp = dest;

	while (*src != '\0') {
		temp[index] = *src;
		temp++;
		src++;
		index++;
	}

	temp[index] = '\0';
	return dest;
}
