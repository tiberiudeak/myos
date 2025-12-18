#include <kernel/multiboot.h>
#include <kernel/font8x16.h>
#include <kernel/string.h>
#include <kernel/tty.h>
#include <kernel/io.h>
#include <mm/vmm.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct tty_operations tty;

static const uint32_t vbe_bg_color = VBE_COLOR_BLACK;
static const uint32_t vbe_fg_color = VBE_COLOR_WHITE;
static const uint8_t vga_bg_color = VGA_COLOR_BLACK;
static const uint8_t vga_fg_color = VGA_COLOR_WHITE;

static uint8_t *font = IBM_VGA_8x16;
//static const struct vbe_mode_info_block *vbe_mode =
//	(struct vbe_mode_info_block *) VBE_MODE_INFO;
static uint32_t *framebuffer;

static size_t terminal_row;
static size_t terminal_column;
static uint32_t terminal_color;

static size_t VBE_WIDTH;
static size_t VBE_HEIGHT;

void _vbe_clear_screen() {
	int num_pixels = VBE_WIDTH * VBE_HEIGHT;

	for (int i = 0; i < num_pixels; i++) {
		framebuffer[i] = vbe_bg_color;
	}
}

void _vbe_terminal_initialize() {

	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vbe_fg_color;

	_vbe_clear_screen();
}

void _vbe_terminal_putentryat(char c, uint32_t color, size_t x, size_t y) {
	// get offset within the font data for the requested character
	int font_offset = c * 16;

	// calculate the starting address of the character's pixel data in the font
	uint8_t *char_data = font + font_offset;

	// calculate the starting address of the character's position in the
	// framebuffer
	uint32_t *dest = framebuffer + (y * VBE_WIDTH * 16 + x * 8);

	for (int row = 0; row < 16; row++) {
		uint8_t pixel_data = char_data[row]; // get the pixel data for this row
		uint32_t *dest_row =
			dest + row * VBE_WIDTH; // calculate the starting address of this
									// row in the framebuffer

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

void _vbe_terminal_scroll() {
	for (size_t y = 0; y < VBE_HEIGHT - 16; ++y) {
		uint32_t *src = framebuffer + (y + 16) * VBE_WIDTH;
		uint32_t *dest = framebuffer + y * VBE_WIDTH;

		for (size_t x = 0; x < VBE_WIDTH; ++x) {
			dest[x] = src[x];
		}
	}

	// clear the bottom 16 rows to background color
	for (size_t y = VBE_HEIGHT - 16; y < VBE_HEIGHT; ++y) {
		uint32_t *dest = framebuffer + y * VBE_WIDTH;

		for (size_t x = 0; x < VBE_WIDTH; ++x) {
			dest[x] = vbe_bg_color;
		}
	}
}

void _vbe_terminal_putchar(char c) {
	unsigned char uc = c;

	if (c == '\t') {
		terminal_column += 1;

		if (terminal_column % 4 != 0) {
			terminal_column += (4 - terminal_column % 4);
		}

		if (terminal_column >= (VBE_WIDTH / 8)) {
			terminal_column = terminal_column - (VBE_WIDTH / 8);

			if (++terminal_row == (VBE_HEIGHT / 16)) {
				_vbe_terminal_scroll();
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
			_vbe_terminal_scroll();
		}

		// set_cursor(terminal_column, terminal_row);

		return;
	}

	_vbe_terminal_putentryat(uc, terminal_color, terminal_column, terminal_row);

	if (++terminal_column == (VBE_WIDTH / 8)) {
		terminal_column = 0;

		if (++terminal_row == (VBE_HEIGHT / 16)) {
			_vbe_terminal_scroll();
			terminal_row = (VBE_HEIGHT / 16) - 1;
		}
	}

	// set_cursor(terminal_column, terminal_row);
}

void _vbe_terminal_write(const char *data, size_t size) {
	for (size_t i = 0; i < size; i++) {
		_vbe_terminal_putchar(data[i]);
	}
}

void _vbe_terminal_writestring(const char *data) {
	_vbe_terminal_write(data, strlen(data));
}

void _vbe_terminal_backspace_cursor(char c) {
	terminal_column--;
	_vbe_terminal_putentryat(c, vbe_bg_color, terminal_column, terminal_row);
}

struct tty_operations vbe_tty_operations = {
	.terminal_initialize		= _vbe_terminal_initialize,
	.terminal_write				= _vbe_terminal_write,
	.terminal_writestring		= _vbe_terminal_writestring,
	.terminal_putchar			= _vbe_terminal_putchar,
	.terminal_backspace_cursor	= _vbe_terminal_backspace_cursor
};

int tty_init_vbe(void *data) {
	if (!data) {
		return 1;
	}

	struct multiboot_info *mbi = (struct multiboot_info *) data;

	uint32_t vaddr = vmm_map_video_mem(mbi->framebuffer_addr,
			mbi->framebuffer_width * mbi->framebuffer_height * (mbi->framebuffer_bpp / 8));

	if (!vaddr) {
		return 1;
	}

	framebuffer = (uint32_t *) vaddr;
	VBE_WIDTH = mbi->framebuffer_width;
	VBE_HEIGHT = mbi->framebuffer_height;

	tty = vbe_tty_operations;

	return 0;
}

static uint16_t *terminal_buffer;

/**
 * @brief Create a VGA color attribute combining foreground and background
 * colors.
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
static inline uint8_t _vga_entry_color(enum vga_color fg, enum vga_color bg) {
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
static inline uint16_t _vga_entry(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}

/**
 * @brief Set the cursor to the specified position.
 *
 * This function sets the cursor to the specified position.
 *
 * @param x  The x position of the cursor.
 * @param y  The y position of the cursor.
 */
void _vga_set_cursor(size_t x, size_t y) {
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
void _vga_terminal_initialize() {
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = _vga_entry_color(vga_fg_color, vga_bg_color); // 0x0F

	_vga_set_cursor(terminal_column, terminal_row);

	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = _vga_entry(' ', (uint8_t) terminal_color);
		}
	}
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
void _vga_terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = _vga_entry(c, color);
}

/**
 * @brief Scroll the terminal.
 *
 * This function scrolls the terminal by one line.
 */
void _vga_terminal_scroll() {
	size_t x, y;

	// scroll all lines up
	for (y = 0; y < VGA_HEIGHT - 1; y++) {
		for (x = 0; x < VGA_WIDTH; x++) {
			terminal_buffer[y*VGA_WIDTH + x] =
				terminal_buffer[(y+1)*VGA_WIDTH + x];
		}
	}

	// clear the last line
	for (x = 0; x < VGA_WIDTH; x++) {
		terminal_buffer[(VGA_HEIGHT-1) * VGA_WIDTH + x] =
			_vga_entry(' ', (uint8_t) terminal_color);
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
void _vga_terminal_putchar(char c) {
	unsigned char uc = c;

	if (c == '\t') {
		terminal_column += 1;

		if (terminal_column % 4 != 0) {
			terminal_column += (4 - terminal_column % 4);
		}

		if (terminal_column >= VGA_WIDTH) {
			terminal_column = terminal_column - VGA_WIDTH;

			if (++terminal_row == VGA_HEIGHT) {
				_vga_terminal_scroll();
				terminal_row = VGA_HEIGHT - 1;
			}
		}

		_vga_set_cursor(terminal_column, terminal_row);
		return;
	}

	if (c == '\n') {
		terminal_column = 0;

		if (++terminal_row == VGA_HEIGHT) {
			terminal_row = VGA_HEIGHT - 1;
			_vga_terminal_scroll();
		}

		_vga_set_cursor(terminal_column, terminal_row);

		return;
	}

	_vga_terminal_putentryat(uc, (uint8_t) terminal_color, terminal_column, terminal_row);

	if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;

		if (++terminal_row == VGA_HEIGHT) {
			_vga_terminal_scroll();
			terminal_row = VGA_HEIGHT - 1;
		}
	}

	_vga_set_cursor(terminal_column, terminal_row);
}

/**
 * @brief Write a stringwith the given size to the terminal
 *
 * This function writes a string with the given size to the terminal.
 *
 * @param data  The string to be displayed.
 * @param size  The size of the string to be displayed.
 */
void _vga_terminal_write(const char *data, size_t size) {
	size_t i;

	for (i = 0; i < size; i++) {
		_vga_terminal_putchar(data[i]);
	}
}

/**
 * @brief Write a string to the terminal
 *
 * This function writes a string to the terminal.
 *
 * @param data  The string to be displayed.
 */
void _vga_terminal_writestring(const char *data) {
	_vga_terminal_write(data, strlen(data));
}

/**
 * @brief Move the cursor back one space
 *
 * This function moves the cursor back one space.
 */
void _vga_terminal_backspace_cursor(char c) {
	(void)c; // eliminate compilation warning
	--terminal_column;
	_vga_terminal_writestring(" ");
	_vga_set_cursor(--terminal_column, terminal_row);
}

struct tty_operations vga_tty_operations = {
	.terminal_initialize		= _vga_terminal_initialize,
	.terminal_write				= _vga_terminal_write,
	.terminal_writestring		= _vga_terminal_writestring,
	.terminal_putchar			= _vga_terminal_putchar,
	.terminal_backspace_cursor	= _vga_terminal_backspace_cursor
};

int tty_init_vga() {
	tty = vga_tty_operations;

	// map video memory (fits in one page table)
	uint32_t vaddr = vmm_map_video_mem(VGA_VIDEO_ADDR, PAGE_SIZE);

	if (!vaddr) {
		return 1;
	}

	terminal_buffer = (uint16_t *) vaddr;

	return 0;
}

/**
 * Write given length of padding
 *
 * @param length	Length of padding to write
 * @param flag		1 = ' ', 0 = '0'
 */
void _tty_print_padding(size_t length, int flag) {
	// allocate string statically, as we can't be sure that
	// the memory manager has been initialized at this point
	// max padding is 16, this should be enough for numbers,
	// could be better for strings, but for now it is what it is
	const char padding_str[16] = "               ";
	const char padding_nr[16] = "000000000000000";
	size_t len = 0;

	if (length > 0) {
		len = length > 16 ? 16 : length;
		if (flag) {
			tty.terminal_write(padding_str, len);
		} else {
			tty.terminal_write(padding_nr, len);
		}
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
int printk(const char *restrict format, ...) {
	va_list parameters;
	va_start(parameters, format);

	int written = 0;
	size_t precision = 0, width = 0;

	while (*format != '\0') {
		if (format[0] != '%') {
			// go through the string until a '%'
			size_t index = 0;
			while (format[index] && format[index] != '%') {
				index++;
			}

			// print the string until the '%'
			tty.terminal_write(format, index);

			format += index;
			written += index;
			continue;
		}

		// skip the '%' character
		format++;
		precision = 0;
		width = 0;

resume:
		if (*format >= '0' && *format <= '9') {
			// determine width
			// read until a '.' or a letter
			size_t index = 0;
			while (format[index] && format[index] != '.' &&
					!(format[index] >= 'a' && format[index] <= 'z')) {
				width = width * 10 + (format[index] - '0');
				index++;
			}

			format += index;
			goto resume;
		} else if (*format == '.') {
			// determine precision
			// skip '.'
			format++;

			// read until a letter
			size_t index = 0;
			while (format[index] && !(format[index] >= 'a' &&
					format[index] <= 'z')) {
				precision = precision * 10 + (format[index] - '0');
				index++;
			}

			format += index;
			goto resume;
		} else if (*format == 's') {
			format++;
			const char *str = va_arg(parameters, const char *);
			size_t len = strlen(str);
			size_t actual_len = (len > precision && precision > 0)
				? precision : len;

			if (width > 0 && actual_len < width) {
				_tty_print_padding(width - actual_len, 1);
			}

			tty.terminal_write(str, (len > precision && precision > 0)
					? precision : len);

			written += len;
		} else if (*format == 'c') {
			format++;
			char c = (char) va_arg(parameters, int);

			if (width > 1) {
				_tty_print_padding(width - 1, 0);
			}

			tty.terminal_write(&c, sizeof(c));

			written++;
		} else if (*format == 'd') {
			format++;
			int i = va_arg(parameters, int);
			char str[12];
			itoa(i, str, 10);

			size_t len = strlen(str);

			if (width > 0 && len < width) {
				_tty_print_padding(width - len, 0);
			}

			tty.terminal_write(str, len);

			written += len;
		} else if (*format == 'x') {
			format++;
			unsigned int i = va_arg(parameters, unsigned int);
			char str[20] = {0};
			itoa(i, str, 16);

			size_t len = strlen(str);

			if (width > 0 && len < width) {
				_tty_print_padding(width - len, 0);
			}

			tty.terminal_write(str, len);

			written += len;
		} else if (*format == 'f') {
			format++;
			float d = va_arg(parameters, double);
			char str[20] = {0};
			ftoa(d, str, 2);

			size_t len = strlen(str);

			if (width > 0 && len < width) {
				_tty_print_padding(width - len, 0);
			}

			tty.terminal_write(str, len);

			written += len;
		} else if (*format == 'l') {
			format++;

			if (*format == 'd') {
				format++;
				long int i = va_arg(parameters, long int);
				char str[20];
				itoa(i, str, 10);

				size_t len = strlen(str);

				if (width > 0 && len < width) {
					_tty_print_padding(width - len, 0);
				}

				tty.terminal_write(str, len);

				written += len;
			} else if (*format == 'x') {
				format++;
				long unsigned int i = va_arg(parameters, long unsigned int);
				char str[20] = {0};
				itoa(i, str, 16);

				size_t len = strlen(str);

				if (width > 0 && len < width) {
					_tty_print_padding(width - len, 0);
				}

				tty.terminal_write(str, len);

				written += len;
			} else if (*format == 'l') {
				format++;

				if (*format == 'd') {
					format++;
					long long int i = va_arg(parameters, long long int);
					char str[20];
					itoa(i, str, 10);

					size_t len = strlen(str);

					if (width > 0 && len < width) {
						_tty_print_padding(width - len, 0);
					}

					tty.terminal_write(str, len);

					written += len;
				} else if (*format == 'x') {
					format++;
					long long unsigned int i =
						va_arg(parameters, long long unsigned int);
					char str[20] = {0};
					itoa(i, str, 16);

					size_t len = strlen(str);

					if (width > 0 && len < width) {
						_tty_print_padding(width - len, 0);
					}

					tty.terminal_write(str, len);

					written += len;
				}
			}
		} else {
			// unsupported format
			return -1;
		}
	}

	va_end(parameters);
	return written;
}
