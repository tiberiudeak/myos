#include <string.h>

int memcmp(const void *ptr1, const void *ptr2, size_t n) {
	if (n == 0 || ptr1 == ptr2) {
		return 0;
	}

	const char *temp1 = ptr1;
	const char *temp2 = ptr2;

	for (size_t i = 0; i < n; i++) {
		if (temp1[i] > temp2[i]) {
			return 1;
		} else if (temp1[i] < temp2[i]) {
			return -1;
		}
	}

	return 0;
}
