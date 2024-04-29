#ifndef _KERNEL_TTYP_H
#define _KERNEL_TTYP_H 1

#include <stddef.h>
#include <stdint.h>

#ifdef CONFIG_TTY_VBE
typedef struct {
    uint16_t attributes;
    uint8_t window_a;
    uint8_t window_b;
	uint16_t granularity;
	uint16_t window_size;
	uint16_t segment_a;
	uint16_t segment_b;
	uint32_t win_func_ptr;
	uint16_t pitch;                     // number of bytes per horizontal line
	uint16_t width;                     // width in pixels
	uint16_t height;                    // height in pixels
	uint8_t w_char;
	uint8_t y_char;
	uint8_t planes;
	uint8_t bpp;                        // bits per pixel
	uint8_t banks;
	uint8_t memory_model;
	uint8_t bank_size;
	uint8_t image_pages;
	uint8_t reserved0;
 
	uint8_t red_mask;
	uint8_t red_position;
	uint8_t green_mask;
	uint8_t green_position;
	uint8_t blue_mask;
	uint8_t blue_position;
	uint8_t reserved_mask;
	uint8_t reserved_position;
	uint8_t direct_color_attributes;
 
	uint32_t framebuffer;               // physical address of the linear frame buffer
	uint32_t off_screen_mem_off;
	uint16_t off_screen_mem_size;
	uint8_t reserved1[206];
} __attribute__ ((packed)) vbe_mode_info_block;

// VBE version of the VGA colors (the fformat is AARRGGBB: alpha, blue, green, red)
typedef enum vbe_colors {
    VBE_COLOR_BLACK             = 0xFF000000,
    VBE_COLOR_BLUE              = 0xFF0000FF,
    VBE_COLOR_GREEN             = 0xFF00FF00,
    VBE_COLOR_CYAN              = 0xFF00FFFF,
    VBE_COLOR_RED               = 0xFFFF0000,
    VBE_COLOR_MAGENTA           = 0xFFFF00FF,
    VBE_COLOR_BROWN             = 0xFFA52A2A,
    VBE_COLOR_LIGHT_GREY        = 0xFFC0C0C0,
    VBE_COLOR_DARK_GREY         = 0xFF808080,
    VBE_COLOR_LIGHT_BLUE        = 0xFFADD8E6,
    VBE_COLOR_LIGHT_GREEN       = 0xFF90EE90,
    VBE_COLOR_LIGHT_CYAN        = 0xFFE0FFFF,
    VBE_COLOR_LIGHT_RED         = 0xFFFFC0CB,
    VBE_COLOR_LIGHT_MAGENTA     = 0xFFFFB6C1,
    VBE_COLOR_LIGHT_BROWN       = 0xFFCD853F,
    VBE_COLOR_WHITE             = 0xFFFFFFFF
} vbe_colors;

uint8_t map_framebuffer(void);
void draw_square(int, int, int, int, uint32_t);
#endif /* CONFIG_TTY_VBE */

void terminal_initialize(void);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char *data);
void terminal_setcolor(uint8_t color);
void terminal_backspace_cursor(char);
int printk(const char* __restrict, ...);
int printkc(int, const char* __restrict, ...);


#endif // _KERNEL_TTYP_H
