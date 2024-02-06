#include <string.h>

char *hextoa(int n, char *str) {
	int i = 0;
	int sign = n;

	if (n < 0) {
		n = -n;
	}

	do {
		int rem = n % 16;
		str[i++] = (rem < 10) ? rem + '0' : rem + 'A' - 10;
	} while ((n /= 16) > 0);

	if (sign < 0) {
		str[i++] = '-';
	}

	str[i] = 'x';
	str[i + 1] = '0';
	str[i + 2] = '\0';
	reverse(str);
	return str;
}
