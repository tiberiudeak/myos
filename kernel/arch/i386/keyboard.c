#include <arch/i386/irq.h>
#include <arch/i386/isr.h>
#include <arch/i386/ps2.h>
#include <kernel/io.h>
#include <kernel/keyboard.h>
#include <kernel/shell.h>
#include <kernel/tty.h>

void keyboard_handler(struct interrupt_regs *r) {
	(void) r; // remove compilation warning

	/* TODO: check what scancode to use */

	/* scancode 1 */
	struct key_info *key_info = (struct key_info *) KEY_INFO_ADDRESS;
	key_info->scancode = 0;

	uint8_t scancode = port_byte_in(PS2_DATA_PORT);

	if (scancode) {
		if (scancode == LSH || scancode == RSH) {
			key_info->shift = 1;
		} else if (scancode == LSH_RELEASE || scancode == RSH_RELEASE) {
			key_info->shift = 0;
		} else {
			if (key_info->shift) {
				scancode = scancode_map_shifted[scancode];
			} else {
				scancode = scancode_map[scancode];
			}

			// send key info to shell
			if (scancode != INV) {
				shell_scancode(scancode);
			}
		}
	}
}

/**
 * @brief Initialize keyboard
 *
 * This function sets the handler of IRQ1 to the keyboard handler
 * function.
 */
void keyboard_init() {
	void (*key_handler)(struct interrupt_regs *r) = keyboard_handler;
	irq_install_handler(1, key_handler);
}
