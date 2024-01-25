#include <stdio.h>

/**
 * @brief Write a string to stdout.
 *
 * This function writes the string pointed to by str to stdout and appends a
 * newline character ('\n') to the end.
 *
 * @param str  The string to be written.
 */
int puts(const char* str) {
	return printf("%s\n", str);
}
