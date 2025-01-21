#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

// printf buffer
char printf_buffer[PRINTF_BUFFER_SIZE] = {0};
uint8_t printf_index = 0;


/**
 * @brief Empty printf buffer and print its content to the stdout stream
 *
 * This function prints the bytes in the printf buffer using the write syscall
 * and then empties the buffer.
 *
 * @return EOF if error occured, 0 otherwise
 */
int fflush(void) {
    printf_buffer[printf_index++] = '\0';
    size_t bytes_written = write(stdout, printf_buffer, printf_index);


    memset(printf_buffer, 0, printf_index);
    printf_index = 0;

    if (bytes_written != printf_index)
        return EOF;

    return 0;
}

/**
 * @brief Add given length from string to the printf buffer
 *
 * This function copies the first len bytes from the given string into to
 * printf buffer.
 *
 * @param str   String to copy from
 * @param len   Length in bytes to copy
 */
void add_str_to_buffer(const char *str, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf_buffer[printf_index++] = str[i];
    }
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
    printf_index = 0;

	while (*format != '\0') {
		if (format[0] != '%') {
			// go through the string until a '%'
			size_t index = 0;
			while (format[index] != '\0' && format[index] != '%') {
				index++;
			}

            add_str_to_buffer(format, index);

            if (format[index] == '\0') {
                written += index;
                break;
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

            add_str_to_buffer(str, len);

			written += len;
		}
		else if (*format == 'c') {
			format++;
			char c = (char) va_arg(parameters, int);

            add_str_to_buffer(&c, sizeof(c));

			written++;
		}
		else if (*format == 'd') {
			format++;
			int i = va_arg(parameters, int);
			char str[12];
			itoa(i, str, 10);

			size_t len = strlen(str);
            add_str_to_buffer(str, len);

			written += len;
		}
		else if (*format == 'x') {
			format++;
			unsigned int i = va_arg(parameters, unsigned int);
			char str[20] = {0};
			itoa(i, str, 16);

			size_t len = strlen(str);
            add_str_to_buffer(str, len);

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
                add_str_to_buffer(str, len);

				written += len;
			}
			else if (*format == 'x') {
				format++;
				long unsigned int i = va_arg(parameters, long unsigned int);
				char str[20] = {0};
				itoa(i, str, 16);

				size_t len = strlen(str);
                add_str_to_buffer(str, len);

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
                    add_str_to_buffer(str, len);

					written += len;
				}
				else if (*format == 'x') {
					format++;
					long long unsigned int i = va_arg(parameters, long long unsigned int);
					char str[20] = {0};
					itoa(i, str, 16);

					size_t len = strlen(str);
                    add_str_to_buffer(str, len);

					written += len;
				}
			}
		}
		else {
			// unsupported format
			return -1;
		}
	}

    fflush();

	va_end(parameters);
	return written;
}
