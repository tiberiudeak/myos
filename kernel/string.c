#include <kernel/string.h>

int atoi(char *str, int *error) {
	int res = 0;
	int sign = 1;
	int i = 0;

	while (str[i] == ' ' || str[i] == '\t' || str[i] == '\n') {
		++i;
	}

	// handle sign
	if (str[i] == '-') {
		sign = -1;
		++i;
	}
	else if (str[i] == '+') {
		++i;
	}

	for (; str[i] != '\0'; ++i) {
		// check if char is not a number
		if (str[i] < '0' || str[i] > '9') {
            *error = 1;
            return sign * res;
		}

		res = res * 10 + str[i] - '0';
	}

    *error = 0;
	return sign * res;
}

// convert integer to string
void intToStr(int x, char *str, int d) {
    int i = 0;
    if (x == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    while (x) {
        int rem = x % 10;
        str[i++] = rem + '0';
        x = x / 10;
    }

    while (i < d) {
        str[i++] = '0';
    }

    str[i] = '\0';

    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

// pow function
float pow(float x, int y) {
    float res = 1.0;
    for (int i = 0; i < y; i++) {
        res *= x;
    }
    return res;
}

char *ftoa(float n, char *str, int afterpoint) {
    int isNegative = 0;

    if (n < 0) {
        isNegative = 1;
        n = -n;
    }

    int exponent = 0;
    int mantissa = 0;

    // extract exponent: 8 bits from 23 to 30
    exponent = (*(int *)&n >> 23) & 0xFF;

    // extract mantissa: 23 bits from 0 to 22
    mantissa = *(int *)&n & 0x7FFFFF;

    // convert exponent to decimal
    int exp = exponent - 127;

    // convert mantissa to decimal
    float dec = 0.0;
    for (int i = 0; i < 23; i++) {
        if (mantissa & (1 << (22 - i))) {
            dec += 1.0 / (1 << (i + 1));
        }
    }

    // calculate the final number
    float result = (1 + dec) * pow(2, exp);

    // convert the number to string
    intToStr((int)result, str, 0);

    // add the decimal point
    if (afterpoint != 0) {
        int len = strlen(str);
       str[len] = '.';
       str[len + 1] = '\0';

        // convert the decimal part to string
        float decimal = result - (int)result;
        for (int i = 0; i < afterpoint; i++) {
            decimal *= 10;
            str[len + 1 + i] = '0' + (int)decimal;
            decimal -= (int)decimal;
        }
        str[len + 1 + afterpoint] = '\0';
    }

    if (isNegative) {
        for (int i = strlen(str); i >= 0; i--) {
            str[i + 1] = str[i];
        }
        str[0] = '-';
    }

    return str;
}


/**
 * @brief Convert integer to string
 *
 * This function converts the given integer to string. If base is 10 and
 * the number is negative, then the resulting string representation will
 * have a minus sign. For any other base, the number is considered unsigned.
 *
 * @param n		Number to be converted to string
 * @param str	Array in memory where to store the resulting null-terminated string
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
	}
	else if (base == 16) {
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
	}
	else {
		// unsupported base
		return NULL;
	}
}

int memcmp(const void *ptr1, const void *ptr2, size_t n) {
	if (n == 0 || ptr1 == ptr2) {
		return 0;
	}

	const char *temp1 = ptr1;
	const char *temp2 = ptr2;

	for (size_t i = 0; i < n; i++) {
		if (temp1[i] > temp2[i]) {
			return 1;
		}
		else if (temp1[i] < temp2[i]) {
			return -1;
		}
	}

	return 0;
}

void *memcpy(void *dest, const void *src, size_t n) {
	char *d = dest;
	const char *s = src;

	for (size_t i = 0; i < n; i++) {
		d[i] = s[i];
	}

	return dest;
}

void *memmove(void *dest, const void *src, size_t n) {
	char *d = dest;
	const char *s = src;

	if (d < s) {
		while (n--) {
			*d++ = *s++;
		}
	}
	else {
		size_t i = 0;

		while (i < n) {
			d[n - i - 1] = s[n - i - 1];
			i++;
		}
	}

	return dest;
}
// #pragma GCC push_options
// #pragma GCC optimize ("01")


void *memset(void *dest, int cons, size_t n) {
	char *d = dest;

	for (size_t i = 0; i < n; i++) {
		d[i] = cons;
	}

	return dest;
}

// #pragma GCC pop_options

void reverse(char *src) {
	int length = strlen(src);

	for (int i = 0; i < length / 2; i++) {
		char temp = src[i];
		src[i] = src[length - i - 1];
		src[length - i - 1] = temp;
	}
}

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

char *strchr(const char *str, int c) {
  char *temp = (char *)str;

  while (*temp != '\0') {
    if (*temp == c) {
      return temp;
    }

    temp++;
  }

  if (c == '\0') {
    return temp;
  }

  return NULL;
}

int strcmp(const char *s1, const char *s2) {
	while (*s1 != '\0' && *s2 != '\0' && *s1 == *s2) {
		s1++;
		s2++;
	}

	return *s1 - *s2;
}

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

size_t strlen(const char *str) {
	size_t len = 0;

	while (str[len] != '\0') {
		len++;
	}

	return len;
}

char *strncat(char *dest, const char *src, size_t n) {
	size_t index = strlen(dest);
	char *temp = dest;

	for (size_t i = 0; i < n && src[i] != '\0'; i++) {
		temp[index + i] = src[i];
	}

	temp[index] = '\0';
	return dest;
}

int strncmp(const char *s1, const char *s2, size_t n) {
	while (n > 0 && *s1 != '\0' && *s2 != '\0' && *s1 == *s2) {
		s1++;
		s2++;
		n--;
	}

    if (n == 0) {
        return 0;
    }

	return *s1 - *s2;
}

char * strncpy(char *dest, const char *src, size_t n) {
	char *temp = dest;
	size_t i;

	for (i = 0; i < n && src[i] != '\0'; i++) {
		temp[i] = src[i];
	}

	for (; i < n; i++) {
		temp[i] = '\0';
	}

	return dest;
}

char *strrchr(const char *str, int c) {
  char *temp = (char *)str;
  char *addr = NULL;

  while (*temp != '\0') {
    if (*temp == c) {
      addr = temp;
    }

    temp++;
  }

  if (c == '\0') {
    return temp;
  }

  return addr;
}

char *strrstr(const char *str1, const char *str2) {
  char *addr = NULL;

  while (*str1 != '\0') {
    if (*str1 == *str2) {
      char *temp1 = (char *)str1;
      char *temp2 = (char *)str2;

      while (*temp1 == *temp2 && *temp1 != '\0') {
        temp1++;
        temp2++;
      }

      if (*temp2 == '\0') {
        addr = (char *)str1;
      }
    }
    str1++;
  }

  return addr;
}

char *strstr(const char *str1, const char *str2) {
  while (*str1 != '\0') {
    if (*str1 == *str2) {
      char *temp1 = (char *)str1;
      char *temp2 = (char *)str2;

      while (*temp1 == *temp2 && *temp1 != '\0') {
        temp1++;
        temp2++;
      }

      if (*temp2 == '\0') {
        return (char *)str1;
      }
    }
    str1++;
  }

  return NULL;
}
