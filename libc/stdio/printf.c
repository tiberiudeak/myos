#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>

/**
 * @brief Write a string of given length to stdout.
 *
 * This function writes length characters from the string pointed to by
 * data to stdout
 *
 * @param data    The string to be written.
 * @param length  The number of characters to be written.
 *
 * @return True if the string was written successfully, false otherwise.
 */
static bool print (const char* data, size_t length) {
	const unsigned char* bytes = (const unsigned char*) data;

	for (size_t i = 0; i < length; i++) {
		if (putchar(bytes[i]) == EOF) {
			return false;
		}
	}

	return true;
}

/**
 * @brief Write a formatted string to stdout.
 *
 * This function writes a formatted string to stdout.
 *
 * @param format  The format string.
 * @param ...     The arguments to be formatted.
 *
 * @return The number of characters written, or a negative value if an error
 * 	   occurred.
 */
int printf(const char* restrict format, ...) {
	va_list parameters;
	va_start(parameters, format);

	int written = 0;

	while (*format != '\0') {
		if (format[0] != '%') {
			// go through the string until a '%'
			size_t index = 0;
			while (format[index] && format[index] != '%') {
				index++;
			}

			// print the string until the '%'
			if (!print(format, index)) {
				return -1;
			}

			format += index;
			written += index;
			continue;
		}

		// skip the '%' character
		format++;

		if (*format == 's') {
			format++;
			const char* str = va_arg(parameters, const char*);
			size_t len = strlen(str);

			if (!print(str, len)) {
				return -1;
			}

			written += len;
		}
		else if (*format == 'c') {
			format++;
			char c = (char) va_arg(parameters, int);

			if (!print(&c, sizeof(c))) {
				return -1;
			}

			written++;
		}
		else if (*format == 'd') {
			format++;
			int i = va_arg(parameters, int);
			char str[12];
			itoa(i, str, 10);

			size_t len = strlen(str);
			if (!print(str, len)) {
				return -1;
			}

			written += len;
		}
		else if (*format == 'x') {
			format++;
			unsigned int i = va_arg(parameters, unsigned int);
			char str[20] = {0};
			itoa(i, str, 16);

			size_t len = strlen(str);
			if (!print(str, len)) {
				return -1;
			}

			written += len;
		}
		else if (*format == 'l') {
			format++;

			if (*format == 'd') {
				format++;
				long int i = va_arg(parameters, long int);
				char str[20];
				itoa(i, str, 10);

				size_t len = strlen(str);
				if (!print(str, len)) {
					return -1;
				}

				written += len;
			}
			else if (*format == 'x') {
				format++;
				long unsigned int i = va_arg(parameters, long unsigned int);
				char str[20] = {0};
				itoa(i, str, 16);

				size_t len = strlen(str);
				if (!print(str, len)) {
					return -1;
				}

				written += len;
			}
			else if (*format == 'l') {
				format++;

				if (*format == 'd') {
					format++;
					long long int i = va_arg(parameters, long long int);
					char str[20];
					itoa(i, str, 10);

					size_t len = strlen(str);
					if (!print(str, len)) {
						return -1;
					}

					written += len;
				}
				else if (*format == 'x') {
					format++;
					long long unsigned int i = va_arg(parameters, long long unsigned int);
					char str[20] = {0};
					itoa(i, str, 16);

					size_t len = strlen(str);
					if (!print(str, len)) {
						return -1;
					}

					written += len;
				}
			}
		}
		else {
			// unsupported format
			return -1;
		}
	}

	va_end(parameters);
	return written;
}
