#include <string.h>

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
	exponent = (*(int *) &n >> 23) & 0xFF;

	// extract mantissa: 23 bits from 0 to 22
	mantissa = *(int *) &n & 0x7FFFFF;

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
	intToStr((int) result, str, 0);

	// add the decimal point
	if (afterpoint != 0) {
		int len = strlen(str);
		str[len] = '.';
		str[len + 1] = '\0';

		// convert the decimal part to string
		float decimal = result - (int) result;
		for (int i = 0; i < afterpoint; i++) {
			decimal *= 10;
			str[len + 1 + i] = '0' + (int) decimal;
			decimal -= (int) decimal;
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
