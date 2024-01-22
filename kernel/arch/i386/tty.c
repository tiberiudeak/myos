#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "../../include/kernel/tty.h"
#include "vga.h"

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xB8000;

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

/**
 * @brief Initialize the terminal.
 *
 * This function initializes the terminal by clearing the screen and setting the
 * cursor to the top-left corner.
 */
void terminal_initialize(void) {
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK); // 0x0F
	terminal_buffer = VGA_MEMORY;

	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

/**
 * @brief Set the color of the terminal.
 *
 * This function sets the color of the terminal.
 *
 * @param color  The color to set the terminal to (8 bits).
 */
void terminal_setcolor(uint8_t color) {
	terminal_color = color;
}

/**
 * @brief Put a character on the terminal.
 *
 * This function puts a character on the terminal at the specified position.
 *
 * @param c      The ASCII character to be displayed (8 bits).
 * @param color  The color to set the character to (8 bits).
 * @param x      The x position of the character
 * @param y      The y position of the character
 */
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

// void terminal_scroll(int line) {
// 	int loop;
// 	char c;

// 	for (loop = line * (VGA_WIDTH * 2) + 0xB8000; loop < (VGA_WIDTH * 2); loop++) {
// 		c = *loop;
// 		*(loop - (VGA_WIDTH * 2)) = c;
// 	}
// }

// void terminal_delete_last_line() {
// 	int x, *ptr;

// 	for (x = 0; x < VGA_WIDTH * 2; x++) {
// 		ptr = 0xB8000 + (VGA_WIDTH * 2 * (VGA_HEIGHT - 1)) + x;
// 		*ptr = 0;
// 	}
// }

/**
 * @brief Put a character on the terminal using terminal_putentryat().
 *
 * This function puts a character on the terminal using terminal_putentryat().
 * It also handles scrolling and deleting the last line if needed.
 *
 * @param c  The ASCII character to be displayed (8 bits).
 */
void terminal_putchar(char c) {
	// int line;
	unsigned char uc = c;

	terminal_putentryat(uc, terminal_color, terminal_column, terminal_row);

	if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;

		if (++terminal_row == VGA_HEIGHT) {
			// for (line = 1; line <= VGA_HEIGHT - 1; line++) {
				// terminal_scroll(line);
				terminal_row = 0;
			// }

			// terminal_delete_last_line();
			// terminal_row = VGA_HEIGHT - 1;
		}
	}
}

/**
 * @brief Write a stringwith the given size to the terminal
 *
 * This function writes a string with the given size to the terminal.
 *
 * @param data  The string to be displayed.
 * @param size  The size of the string to be displayed.
 */
void terminal_write(const char* data, size_t size) {
	size_t i;

	for (i = 0; i < size; i++) {
		terminal_putchar(data[i]);
	}
}

/**
 * @brief Write a string to the terminal
 *
 * This function writes a string to the terminal.
 *
 * @param data  The string to be displayed.
 */
void terminal_writestring(const char* data) {
	terminal_write(data, strlen(data));
}