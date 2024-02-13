#include <kernel/keyboard.h>
#include <arch/i386/ps2.h>
#include <kernel/io.h>
#include <stdio.h>

void keyboard_handler() {

	/* TODO: check what scancode to use */

	/* scancode 1 */
	key_info_t *key_info = (key_info_t*) KEY_INFO_ADDRESS;
	key_info->scancode = 0;

	uint8_t scancode = port_byte_in(PS2_DATA_PORT);

	if (scancode) {
		if (scancode == LSH || scancode == RSH) {
			key_info->shift = 1;
		}
		else if (scancode == LSH_RELEASE || scancode == RSH_RELEASE) {
			key_info->shift = 0;
		}
		else {
			if (key_info->shift) {
				scancode = scancode_map_shifted[scancode];
			}
			else {
				scancode = scancode_map[scancode];
			}

			// send key info to shell
			char c[2];
			c[0] = scancode;
			c[1] = '\0';
			printf("%s", c);
		}
	}
}

void keyboard_init() {
	irq_install_handler(1, keyboard_handler);
}
