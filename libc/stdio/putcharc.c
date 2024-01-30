#include <stdio.h>

#if defined(__is_libk)
#include <kernel/tty.h>
#endif

/**
 * @brief Write a character to stdout with color.
 *
 * This function writes the character c to stdout with color.
 *
 * @param c  The character to be written.
 * @param color  The color of the character to be written.
 *
 * @return The character written
 */
int putcharc(int ic, int color) {
#if defined(__is_libk)
	char c = (char) ic;
	terminal_setcolor(color);
	terminal_write(&c, sizeof(c));
	terminal_setcolor(15);
#else

#endif
	return ic;
}
