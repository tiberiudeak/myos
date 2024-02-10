#ifndef KERNEL_KEY_H
#define KERNEL_KEY_H 1

#include <arch/i386/irq.h>

#define INV		0x00
#define ESC		0x1B
#define LCTRL	0x00
#define RCTRL	0x00
#define LSH		0x00
#define RSH		0x00
#define LALT	0x00
#define RALT	0x00
#define CAPS	0x00

static uint8_t scancode_map[] =
{
	INV, ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
	'\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', LCTRL,
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', LSH, '\\',
	'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', RSH, '*', LALT, ' ', CAPS,
	INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, '7', '8', '9',
	'-', '4', '5', '6', '+', '1', '2', '3', '0', '.', INV, INV, INV, INV, INV,
	INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
	INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
	INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
	INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
	INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
	INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV
};

void keyboard_handler();
void keyboard_init(void);

#endif /* !KERNEL_KEY_H */
