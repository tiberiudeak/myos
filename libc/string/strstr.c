#include <string.h>

char *strstr(const char *str1, const char *str2) {
	while (*str1 != '\0') {
		if (*str1 == *str2) {
			char *temp1 = (char *) str1;
			char *temp2 = (char *) str2;

			while (*temp1 == *temp2 && *temp1 != '\0') {
				temp1++;
				temp2++;
			}

			if (*temp2 == '\0') {
				return (char *) str1;
			}
		}
		str1++;
	}

	return NULL;
}
