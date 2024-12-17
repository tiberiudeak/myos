#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <kernel/tty.h>
#include <global_addresses.h>

#ifdef CONFIG_SH_FGC_BLACK
    static const uint8_t default_color = 0;
#elif CONFIG_SH_FGC_BLUE
    static const uint8_t default_color = 1;
#elif CONFIG_SH_FGC_GREEN
    static const uint8_t default_color = 2;
#elif CONFIG_SH_FGC_CYAN
    static const uint8_t default_color = 3;
#elif CONFIG_SH_FGC_RED
    static const uint8_t default_color = 4;
#elif CONFIG_SH_FGC_MAGENTA
    static const uint8_t default_color = 5;
#elif CONFIG_SH_FGC_BROWN
    static const uint8_t default_color = 6;
#elif CONFIG_SH_FGC_LIGHT_GREY
    static const uint8_t default_color = 7;
#elif CONFIG_SH_FGC_DARK_GREY
    static const uint8_t default_color = 8;
#elif CONFIG_SH_FGC_LIGHT_BLUE
    static const uint8_t default_color = 9;
#elif CONFIG_SH_FGC_LIGHT_GREEN
    static const uint8_t default_color = 10;
#elif CONFIG_SH_FGC_LIGHT_CYAN
    static const uint8_t default_color = 11;
#elif CONFIG_SH_FGC_LIGHT_RED
    static const uint8_t default_color = 12;
#elif CONFIG_SH_FGC_LIGHT_MAGENTA
    static const uint8_t default_color = 13;
#elif CONFIG_SH_FGC_LIGHT_BROWN
    static const uint8_t default_color = 14;
#else
    static const uint8_t default_color = 15;
#endif

#ifdef CONFIG_TTY_VBE

#ifdef CONFIG_SH_BGC_BLUE
    static const uint32_t background_color = VBE_COLOR_BLUE;
#elif CONFIG_SH_BGC_GREEN
    static const uint32_t background_color = VBE_COLOR_GREEN;
#elif CONFIG_SH_BGC_CYAN
    static const uint32_t background_color = VBE_COLOR_CYAN;
#elif CONFIG_SH_BGC_RED
    static const uint32_t background_color = VBE_COLOR_RED;
#elif CONFIG_SH_BGC_MAGENTA
    static const uint32_t background_color = VBE_COLOR_MAGENTA;
#elif CONFIG_SH_BGC_BROWN
    static const uint32_t background_color = VBE_COLOR_BROWN;
#elif CONFIG_SH_BGC_LIGHT_GREY
    static const uint32_t background_color = VBE_COLOR_LIGHT_GREY;
#elif CONFIG_SH_BGC_DARK_GREY
    static const uint32_t background_color = VBE_COLOR_DARK_GREY;
#elif CONFIG_SH_BGC_LIGHT_BLUE
    static const uint32_t background_color = VBE_COLOR_LIGHT_BLUE;
#elif CONFIG_SH_BGC_LIGHT_GREEN
    static const uint32_t background_color = VBE_COLOR_LIGHT_GREEN;
#elif CONFIG_SH_BGC_LIGHT_CYAN
    static const uint32_t background_color = VBE_COLOR_LIGHT_CYAN;
#elif CONFIG_SH_BGC_LIGHT_RED
    static const uint32_t background_color = VBE_COLOR_LIGHT_RED;
#elif CONFIG_SH_BGC_LIGHT_MAGENTA
    static const uint32_t background_color = VBE_COLOR_LIGHT_MAGENTA;
#elif CONFIG_SH_BGC_LIGHT_BROWN
    static const uint32_t background_color = VBE_COLOR_LIGHT_BROWN;
#elif CONFIG_SH_BGC_WHITE
    static const uint32_t background_color = VBE_COLOR_WHITE;
#else
    static const uint32_t background_color = VBE_COLOR_BLACK;
#endif

#else
#include "vga.h"

#ifdef CONFIG_SH_BGC_BLUE
    static const uint8_t background_color = VGA_COLOR_BLUE;
#elif CONFIG_SH_BGC_GREEN
    static const uint8_t background_color = VGA_COLOR_GREEN;
#elif CONFIG_SH_BGC_CYAN
    static const uint8_t background_color = VGA_COLOR_CYAN;
#elif CONFIG_SH_BGC_RED
    static const uint8_t background_color = VGA_COLOR_RED;
#elif CONFIG_SH_BGC_MAGENTA
    static const uint8_t background_color = VGA_COLOR_MAGENTA;
#elif CONFIG_SH_BGC_BROWN
    static const uint8_t background_color = VGA_COLOR_BROWN;
#elif CONFIG_SH_BGC_LIGHT_GREY
    static const uint8_t background_color = VGA_COLOR_LIGHT_GREY;
#elif CONFIG_SH_BGC_DARK_GREY
    static const uint8_t background_color = VGA_COLOR_DARK_GREY;
#elif CONFIG_SH_BGC_LIGHT_BLUE
    static const uint8_t background_color = VGA_COLOR_LIGHT_BLUE;
#elif CONFIG_SH_BGC_LIGHT_GREEN
    static const uint8_t background_color = VGA_COLOR_LIGHT_GREEN;
#elif CONFIG_SH_BGC_LIGHT_CYAN
    static const uint8_t background_color = VGA_COLOR_LIGHT_CYAN;
#elif CONFIG_SH_BGC_LIGHT_RED
    static const uint8_t background_color = VGA_COLOR_LIGHT_RED;
#elif CONFIG_SH_BGC_LIGHT_MAGENTA
    static const uint8_t background_color = VGA_COLOR_LIGHT_MAGENTA;
#elif CONFIG_SH_BGC_LIGHT_BROWN
    static const uint8_t background_color = VGA_COLOR_LIGHT_BROWN;
#elif CONFIG_SH_BGC_WHITE
    static const uint8_t background_color = VGA_COLOR_WHITE;
#else
    static const uint8_t background_color = VGA_COLOR_BLACK;
#endif

#endif


#ifdef CONFIG_TTY_VBE

#include <global_addresses.h>
#include <mm/vmm.h>

static uint8_t *font = (uint8_t*)VGA_BIOS_FONT;
static const struct vbe_mode_info_block *vbe_mode = (struct vbe_mode_info_block*)VBE_MODE_INFO;
static uint32_t *framebuffer;

static size_t terminal_row;
static size_t terminal_column;
static uint32_t terminal_color;

static size_t VBE_WIDTH;
static size_t VBE_HEIGHT;

void clear_screen(void) {
    int num_pixels = VBE_WIDTH * VBE_HEIGHT;

    for (int i = 0; i < num_pixels; i++) {
        ((uint32_t*)framebuffer)[i] = background_color;
    }
}

void terminal_initialize(void) {
    framebuffer = (uint32_t*)vbe_mode->framebuffer;
    VBE_WIDTH = vbe_mode->width;
    VBE_HEIGHT = vbe_mode->height;

	terminal_row = 0;
	terminal_column = 0;
    terminal_setcolor(default_color);

    clear_screen();
}

void terminal_putentryat(char c, uint32_t color, size_t x, size_t y) {

    // get offset within the font data for the requested character
    int font_offset = c * 16;

    // calculate the starting address of the character's pixel data in the font
    uint8_t* char_data = font + font_offset;

    // calculate the starting address of the character's position in the framebuffer
    uint32_t* dest = framebuffer + (y * VBE_WIDTH * 16 + x * 8);

    for (int row = 0; row < 16; row++) {
        uint8_t pixel_data = char_data[row];            // get the pixel data for this row
        uint32_t* dest_row = dest + row * VBE_WIDTH;    // calculate the starting address of this row in the framebuffer

        // iterate over each pixel in the row
        for (int col = 0; col < 8; col++) {
            // check if the current pixel is set in the pixel data
            if (pixel_data & (1 << (7 - col))) {
                // set the corresponding pixel in the framebuffer
                dest_row[col] = color;
            }
        }
    }
}

void terminal_scroll(void) {
    for (size_t y = 0; y < VBE_HEIGHT - 16; ++y) {
        uint32_t* src = framebuffer + (y + 16) * VBE_WIDTH;
        uint32_t* dest = framebuffer + y * VBE_WIDTH;

        for (size_t x = 0; x < VBE_WIDTH; ++x) {
            dest[x] = src[x];
        }
    }

    // clear the bottom 16 rows to background color
	for (size_t y = VBE_HEIGHT - 16; y < VBE_HEIGHT; ++y) {
		uint32_t* dest = framebuffer + y * VBE_WIDTH;

		for (size_t x = 0; x < VBE_WIDTH; ++x) {
			dest[x] = background_color;
		}
	}
}

void terminal_putchar(char c) {

	unsigned char uc = c;

	if (c == '\t') {
		terminal_column += 1;

		if (terminal_column % 4 != 0) {
			terminal_column += (4 - terminal_column % 4);
		}

		if (terminal_column >= (VBE_WIDTH / 8)) {
			terminal_column = terminal_column - (VBE_WIDTH / 8);

			if (++terminal_row == (VBE_HEIGHT / 16)) {
				terminal_scroll();
				terminal_row = (VBE_HEIGHT / 16) - 1;
			}
		}

		// set_cursor(terminal_column, terminal_row);
		return;
	}

	if (c == '\n') {
		terminal_column = 0;

		if (++terminal_row == (VBE_HEIGHT / 16)) {
			terminal_row = (VBE_HEIGHT / 16) - 1;
			terminal_scroll();
		}

		// set_cursor(terminal_column, terminal_row);

		return;
	}

	terminal_putentryat(uc, terminal_color, terminal_column, terminal_row);


	if (++terminal_column == (VBE_WIDTH / 8)) {
		terminal_column = 0;

		if (++terminal_row == (VBE_HEIGHT / 16)) {
			terminal_scroll();
			terminal_row = (VBE_HEIGHT / 16) - 1;
		}
	}

	// set_cursor(terminal_column, terminal_row);
}

void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        terminal_putchar(data[i]);
    }
}

void terminal_writestring(const char* data) {
    terminal_write(data, strlen(data));
}

void terminal_setcolor(uint8_t color) {
    switch (color) {
        case 0:
            terminal_color = VBE_COLOR_BLACK;
            break;
        case 1:
            terminal_color = VBE_COLOR_BLUE;
            break;
        case 2:
            terminal_color = VBE_COLOR_GREEN;
            break;
        case 3:
            terminal_color = VBE_COLOR_CYAN;
            break;
        case 4:
            terminal_color = VBE_COLOR_RED;
            break;
        case 5:
            terminal_color = VBE_COLOR_MAGENTA;
            break;
        case 6:
            terminal_color = VBE_COLOR_BROWN;
            break;
        case 7:
            terminal_color = VBE_COLOR_LIGHT_GREY;
            break;
        case 8:
            terminal_color = VBE_COLOR_DARK_GREY;
            break;
        case 9:
            terminal_color = VBE_COLOR_LIGHT_BLUE;
            break;
        case 10:
            terminal_color = VBE_COLOR_LIGHT_GREEN;
            break;
        case 11:
            terminal_color = VBE_COLOR_LIGHT_CYAN;
            break;
        case 12:
            terminal_color = VBE_COLOR_LIGHT_RED;
            break;
        case 13:
            terminal_color = VBE_COLOR_LIGHT_MAGENTA;
            break;
        case 14:
            terminal_color = VBE_COLOR_LIGHT_BROWN;
            break;
        case 15:
            terminal_color = VBE_COLOR_WHITE;
            break;
        default:
            terminal_color = VBE_COLOR_WHITE;
    }
}

uint8_t map_framebuffer(void) {
    uint32_t framebuffer_size = vbe_mode->width * vbe_mode->pitch;
    uint32_t framebuffer_size_pages = framebuffer_size / PAGE_SIZE;

    if (framebuffer_size_pages % PAGE_SIZE > 0) {
        framebuffer_size_pages++;
    }

    framebuffer_size_pages *= 2;
    int ret;

    for (uint32_t i = 0, fb_start = vbe_mode->framebuffer; i < framebuffer_size_pages; i++, fb_start += PAGE_SIZE) {
        ret = map_page((void*)fb_start, (void*)fb_start);
        if (ret) {
            return ret;
        }
    }

    return 0;
}

void terminal_backspace_cursor(char c) {
    terminal_column--;
    terminal_putentryat(c, background_color, terminal_column, terminal_row);
}

// void draw_square(int x, int y, int width, int height, uint32_t color) {
//     if (x < 0 || x + width > VBE_WIDTH || y < 0 || y + height > VBE_HEIGHT) {
//         return;
//     }
// 
//     for (int i = y; i < y + height; ++i) {
//         for (int j = x; j < x + width; ++j) {
//             int pixelIndex = i * VBE_WIDTH + j;
// 
//             framebuffer[pixelIndex] = color;
//         }
//     }
// }

#else /* CONFIG_TTY_VBE */

#include <kernel/io.h>

#define REG_SCREEN_CTRL 0x3D4
#define REG_SCREEN_DATA 0x3D5

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xB8000;

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

/**
 * @brief Set the cursor to the specified position.
 *
 * This function sets the cursor to the specified position.
 *
 * @param x  The x position of the cursor.
 * @param y  The y position of the cursor.
 */
void set_cursor(size_t x, size_t y) {
	uint16_t pos = y * VGA_WIDTH + x;

	port_byte_out(REG_SCREEN_CTRL, 14);
	port_byte_out(REG_SCREEN_DATA, pos >> 8);
	port_byte_out(REG_SCREEN_CTRL, 15);
	port_byte_out(REG_SCREEN_DATA, pos);
}

/**
 * @brief Initialize the terminal.
 *
 * This function initializes the terminal by clearing the screen and setting the
 * cursor to the top-left corner.
 */
void terminal_initialize(void) {
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(default_color, background_color); // 0x0F
	terminal_buffer = VGA_MEMORY;

	set_cursor(terminal_column, terminal_row);

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
	terminal_color = vga_entry_color(color, background_color);
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

/**
 * @brief Scroll the terminal.
 *
 * This function scrolls the terminal by one line.
 */
void terminal_scroll(void) {
	size_t x, y;
	uint16_t *ptr;

	// scroll all lines up
	for (y = 0; y < VGA_HEIGHT - 2; y++) {
		for (x = 0; x < VGA_WIDTH * 2; x++) {
			ptr = VGA_MEMORY + (VGA_WIDTH * y) + x;
			*ptr = *(ptr + (VGA_WIDTH));
		}
	}

    // clear the last line
	for (x = 0; x < VGA_WIDTH * 2; x++) {
		ptr = VGA_MEMORY + (VGA_WIDTH * (VGA_HEIGHT - 1)) + x;
		//*ptr = *(ptr + (VGA_WIDTH));
        *ptr = vga_entry(' ', terminal_color);
	}
}

/**
 * @brief Put a character on the terminal using terminal_putentryat().
 *
 * This function puts a character on the terminal using terminal_putentryat().
 * It also handles scrolling and deleting the last line if needed.
 *
 * @param c  The ASCII character to be displayed (8 bits).
 */
void terminal_putchar(char c) {
	unsigned char uc = c;

	if (c == '\t') {
		terminal_column += 1;

		if (terminal_column % 4 != 0) {
			terminal_column += (4 - terminal_column % 4);
		}

		if (terminal_column >= VGA_WIDTH) {
			terminal_column = terminal_column - VGA_WIDTH;

			if (++terminal_row == VGA_HEIGHT) {
				terminal_scroll();
				terminal_row = VGA_HEIGHT - 1;
			}
		}

		set_cursor(terminal_column, terminal_row);
		return;
	}

	if (c == '\n') {
		terminal_column = 0;

		if (++terminal_row == VGA_HEIGHT) {
			terminal_row = VGA_HEIGHT - 1;
			terminal_scroll();
		}

		set_cursor(terminal_column, terminal_row);

		return;
	}

	terminal_putentryat(uc, terminal_color, terminal_column, terminal_row);

	if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;

		if (++terminal_row == VGA_HEIGHT) {
			terminal_scroll();
			terminal_row = VGA_HEIGHT - 1;
		}
	}

	set_cursor(terminal_column, terminal_row);
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

/**
 * @brief Move the cursor back one space
 *
 * This function moves the cursor back one space.
 */
void terminal_backspace_cursor(char c) {
    c++;    // used to eliminate compilation warning
    --terminal_column;
    terminal_writestring(" ");
	set_cursor(--terminal_column, terminal_row);
}

#endif /* CONFIG_TTY_VGA */



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
static bool __print(const char* data, size_t length, uint8_t color) {
    terminal_setcolor(color);
    terminal_write(data, length);
    terminal_setcolor(default_color);  // se color back to default
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
int printkc(int color, const char* restrict format, ...) {
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
			if (!__print(format, index, color)) {
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

			if (!__print(str, len, color)) {
				return -1;
			}

			written += len;
		}
		else if (*format == 'c') {
			format++;
			char c = (char) va_arg(parameters, int);

			if (!__print(&c, sizeof(c), color)) {
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
			if (!__print(str, len, color)) {
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
			if (!__print(str, len, color)) {
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
				if (!__print(str, len, color)) {
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
				if (!__print(str, len, color)) {
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
					if (!__print(str, len, color)) {
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
					if (!__print(str, len, color)) {
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
int printk(const char* restrict format, ...) {
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
            terminal_write(format, index);

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

            terminal_write(str, len);

			written += len;
		}
		else if (*format == 'c') {
			format++;
			char c = (char) va_arg(parameters, int);

            terminal_write(&c, sizeof(c));

			written++;
		}
		else if (*format == 'd') {
			format++;
			int i = va_arg(parameters, int);
			char str[12];
			itoa(i, str, 10);

			size_t len = strlen(str);
            terminal_write(str, len);

			written += len;
		}
		else if (*format == 'x') {
			format++;
			unsigned int i = va_arg(parameters, unsigned int);
			char str[20] = {0};
			itoa(i, str, 16);

			size_t len = strlen(str);
            terminal_write(str, len);

			written += len;
		}
		else if (*format == 'f') {
			format++;
			float d = va_arg(parameters, double);
			char str[20] = {0};
			ftoa(d, str, 2);

			size_t len = strlen(str);
			terminal_write(str, len);

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
                terminal_write(str, len);

				written += len;
			}
			else if (*format == 'x') {
				format++;
				long unsigned int i = va_arg(parameters, long unsigned int);
				char str[20] = {0};
				itoa(i, str, 16);

				size_t len = strlen(str);
                terminal_write(str, len);

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
                    terminal_write(str, len);

					written += len;
				}
				else if (*format == 'x') {
					format++;
					long long unsigned int i = va_arg(parameters, long long unsigned int);
					char str[20] = {0};
					itoa(i, str, 16);

					size_t len = strlen(str);
                    terminal_write(str, len);

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
