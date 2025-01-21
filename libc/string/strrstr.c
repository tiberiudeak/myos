#include <string.h>

char *strrstr(const char *str1, const char *str2) {
	char *addr = NULL;

	while (*str1 != '\0') {
		if (*str1 == *str2) {
			char *temp1 = (char *) str1;
			char *temp2 = (char *) str2;

			while (*temp1 == *temp2 && *temp1 != '\0') {
				temp1++;
				temp2++;
			}

			if (*temp2 == '\0') {
				addr = (char *) str1;
			}
		}
		str1++;
	}

	return addr;
}
