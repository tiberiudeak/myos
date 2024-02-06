#include <string.h>

char *itoa(int n, char *str) {
	int i = 0;
	int sign = n;

	if (n < 0) {
		n = -n;
	}

	do {
		str[i++] = n % 10 + '0';
	} while ((n /= 10) > 0);

	if (sign < 0) {
		str[i++] = '-';
	}

	str[i] = '\0';
	reverse(str);
	return str;
}