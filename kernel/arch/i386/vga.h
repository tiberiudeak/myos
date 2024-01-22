#ifndef ARCH_I386_VGA_H
#define ARCH_I386_VGA_H 1

#include <stdint.h>

enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15
};

/**
 * @brief Create a VGA color attribute combining foreground and background colors.
 *
 * This function takes foreground and background color enums and combines them
 * into a single 8-bit value suitable for use as a color attribute in VGA text
 * mode. The lower 4 bits represent the foreground color, and the upper 4 bits
 * represent the background color.
 *
 * @param fg  The foreground color enum (from enum vga_color).
 * @param bg  The background color enum (from enum vga_color).
 *
 * @return The combined 8-bit value representing the foreground and background
 *         colors for use as a color attribute in VGA text mode.
 */
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
	return fg | bg << 4;
}

/**
 * @brief Create a VGA entry combining an ASCII character and color attribute.
 *
 * This function takes an ASCII character and a color attribute, then combines
 * them into a single 16-bit value suitable for writing to the video memory,
 * where the upper 8 bits represent the color attribute and the lower 8 bits
 * represent the ASCII character.
 *
 * @param uc     The ASCII character to be displayed (8 bits).
 * @param color  The color attribute for the character (8 bits).
 *               The lower 4 bits represent the foreground color, and the
 *               upper 4 bits represent the background color.
 *
 * @return The combined 16-bit value representing the character and color.
 */
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}

#endif // ARCH_I386_VGA_H
