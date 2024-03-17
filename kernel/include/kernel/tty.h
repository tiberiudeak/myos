#ifndef _KERNEL_TTYP_H
#define _KERNEL_TTYP_H 1

#include <stddef.h>
#include <stdint.h>

#ifdef TTY_VGA
void terminal_initialize(void);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char *data);
void terminal_setcolor(uint8_t color);
void terminal_backspace_cursor();
#endif /* TTY_VGA */

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

void terminal_initialize(void);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char *data);
void terminal_setcolor(uint8_t color);
void terminal_backspace_cursor();

#endif // _KERNEL_TTYP_H
