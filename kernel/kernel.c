#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "include/kernel/tty.h"

void test() {
	char* video_memory = (char*) 0xb8000;
	*video_memory = 'A';
	*(video_memory + 2) = 'Y';
}

void kmain() {
	terminal_initialize();
	terminal_writestring("Hello, kernel World!\n");

	// for (size_t i = 0; i < 23; i++) {
		// terminal_writestring("Hello, kernel World!\n");
	// }
}
