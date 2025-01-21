#include <string.h>

/**
 * @brief Convert integer to string
 *
 * This function converts the given integer to string. If base is 10 and
 * the number is negative, then the resulting string representation will
 * have a minus sign. For any other base, the number is considered unsigned.
 *
 * @param n		Number to be converted to string
 * @param str	Array in memory where to store the resulting null-terminated
 * string
 * @param base	Numerical base to represent the number (10 = decimal, 16 = hexa)
 *
 * @return pointer to the resulting string, same as parameter str
 */
char *itoa(int n, char *str, int base) {
	int i = 0;

	if (base == 10) {
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
	} else if (base == 16) {
		unsigned int un = n;

		do {
			int rem = un % 16;
			str[i++] = (rem < 10) ? rem + '0' : rem + 'A' - 10;
		} while ((un /= 16) > 0);

		while (i < 8) {
			str[i++] = '0';
		}

		str[i] = 'x';
		str[i + 1] = '0';
		str[i + 2] = '\0';

		reverse(str);
		return str;
	} else {
		// unsupported base
		return NULL;
	}
}
