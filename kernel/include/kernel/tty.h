#ifndef _KERNEL_TTYP_H
#define _KERNEL_TTYP_H 1

#include <stddef.h>
#include <stdint.h>

void terminal_initialize(void);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char *data);
void terminal_setcolor(uint8_t color);
void terminal_backspace_cursor();

#endif // _KERNEL_TTYP_H
