#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>

/**
 * @brief Write a string of given length to stdout with color.
 *
 * This function writes length characters from the string pointed to by
 * data to stdout with the given color.
 *
 * @param data    The string to be written.
 * @param length  The number of characters to be written.
 * @param color   The color of the string to be written.
 *
 * @return True if the string was written successfully, false otherwise.
 */
static bool print_color(const char* data, size_t length, int color) {
	const unsigned char* bytes = (const unsigned char*) data;

	for (size_t i = 0; i < length; i++) {
		if (putcharc(bytes[i], color) == EOF) {
			return false;
		}
	}

	return true;
}

/**
 * @brief Write a formatted string to stdout with color.
 *
 * This function writes a formatted string to stdout with color.
 *
 * @param color   The color of the string to be written.
 * @param format  The format string.
 * @param ...     The arguments to be formatted.
 *
 * @return The number of characters written, or a negative value if an error
 * 	   occurred.
 */
int printfc(int color, const char *format, ...) {
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
			if (!print_color(format, index, color)) {
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

			if (!print_color(str, len, color)) {
				return -1;
			}

			written += len;
		}
		else if (*format == 'c') {
			format++;
			char c = (char) va_arg(parameters, int);

			if (!print_color(&c, sizeof(c), color)) {
				return -1;
			}

			written++;
		}
	}

	va_end(parameters);
	return written;
}
