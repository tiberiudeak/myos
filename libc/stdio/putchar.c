#include <stdio.h>

#if defined(__is_libk)
#include <kernel/tty.h>
#endif

/**
 * @brief Write a character to stdout.
 *
 * This function writes the character c to stdout.
 *
 * @param c  The character to be written.
 *
 * @return The character written
 */
int putchar(int ic) {
#if defined(__is_libk)
	char c = (char) ic;
	terminal_write(&c, sizeof(c));
#else
	// Write syscall coming soon!
#endif
	return ic;
}
