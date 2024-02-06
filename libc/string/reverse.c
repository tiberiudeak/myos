#include <string.h>

void reverse(char *src) {
	int length = strlen(src);

	for (int i = 0; i < length / 2; i++) {
		char temp = src[i];
		src[i] = src[length - i - 1];
		src[length - i - 1] = temp;
	}
}
