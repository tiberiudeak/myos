#include <string.h>

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
	} else if (str[i] == '+') {
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
