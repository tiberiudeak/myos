#include <kernel/keyboard.h>
#include <arch/i386/ps2.h>
#include <kernel/io.h>
#include <stdio.h>

void keyboard_handler() {
	uint8_t scancode = port_byte_in(PS2_DATA_PORT);
	char c[2];
	c[0] = scancode_map[scancode];
	c[1] = '\0';
	printf("%s", c);
}

void keyboard_init() {
	irq_install_handler(1, keyboard_handler);
}
