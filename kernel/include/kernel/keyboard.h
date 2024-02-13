#ifndef KERNEL_KEY_H
#define KERNEL_KEY_H 1

#include <arch/i386/irq.h>

#define INV					0x00
#define ESC					0x1B
#define LCTRL				0x1D
#define RCTRL				0xE0
#define LSH					0x2A
#define RSH					0x36
#define LALT				0x00
#define RALT				0x00
#define CAPS				0x3A

#define LSH_RELEASE			0xAA
#define RSH_RELEASE			0xB6

/* there is no memory allocator yet, so the
key information will be stored at the fixed address 0x500
*/
#define KEY_INFO_ADDRESS	0x500

/* scancode 1 */
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

static uint8_t scancode_map_shifted[] =
{
	INV, ESC, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
	'\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', LCTRL,
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', LSH, '|',
	'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', RSH, '*', LALT, ' ', CAPS,
	INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, '7', '8', '9',
	'-', '4', '5', '6', '+', '1', '2', '3', '0', '.', INV, INV, INV, INV, INV,
	INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
	INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
	INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
	INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
	INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
	INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV
};

/* TODO: add scancode 2 and 3 */

typedef struct {
	uint8_t scancode;
	uint8_t shift;
	uint8_t ctrl;
	uint8_t caps;
} __attribute__((packed)) key_info_t;

void keyboard_handler();
void keyboard_init(void);

#endif /* !KERNEL_KEY_H */
