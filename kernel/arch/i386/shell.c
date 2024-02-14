#include <kernel/shell.h>
#include <kernel/keyboard.h>
#include <kernel/tty.h>
#include <mm/pmm.h>
#include <string.h>
#include <stdio.h>

static char key_buffer[MAX_COMMAND_LENGTH];
static int index;

void shell_exec_command(char *command) {

	if (strcmp(command, "memmap") == 0) {
		print_mem_map();
	}
	else {
		printf("%s: unknown command\n", command);
	}

	index = 0;
	memset(key_buffer, 0, MAX_COMMAND_LENGTH);

	shell_init();
}

void shell_scancode(uint8_t scancode) {
	if (index >= MAX_COMMAND_LENGTH) {
		return;
	}

	// check if scancode is backspace -> delete last character
	if (scancode == BACKSPACE) {
		if (index > 0) {
			key_buffer[--index] = '\0';

			terminal_backspace_cursor();
			printf(" ");
			terminal_backspace_cursor();
		}
		return;
	}

	// check if scancode is enter -> execute command
	if (scancode == ENTER) {
		printf("\n");
		shell_exec_command(key_buffer);

		return;
	}

	key_buffer[index++] = scancode;

	putchar(scancode);
}

/* for now, the initialization only prints the prompt */
void shell_init() {
	printf("> ");
}
