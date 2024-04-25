#include <kernel/shell.h>
#include <kernel/keyboard.h>
#include <kernel/tty.h>
#include <arch/i386/pit.h>
#include <arch/i386/rtc.h>
#include <process/scheduler.h>
#include <process/process.h>
#include <mm/pmm.h>
#include <mm/kmalloc.h>
#include <elf.h>
#include <fs.h>
#include <string.h>
#include <stdio.h>

static char key_buffer[MAX_COMMAND_LENGTH];
static int index;

void test_task(int argc, char *argv[]) {
    printk("test task %d %s\n", argc, argv[0]);
}

void shell_exec_command(char *command) {

    // TODO: remove spaces at the beginning

	if (strcmp(command, "memmap") == 0) {
		print_mem_map();
	}
	else if (strcmp(command, "uptime") == 0) {
		printk("%d\n", get_uptime());
	}
	else if (strcmp(command, "pmeminfo") == 0) {
		print_phymem_info();
	}
	else if (strcmp(command, "datetime") == 0) {
		rtc_print_datetime();
	}
	else if (strcmp(command, "dl") == 0) {
		print_superblock_info();
	}
	else if (strcmp(command, "ls") == 0) {
		fs_print_dir();
	}
	else if (strncmp(command, "./", 2) == 0) {
        // TODO: parse command into argvs
        char *argv = kmalloc(sizeof(char) * 10);
        strncpy(argv, command, 10);

        // create new task
        task_struct *new_task = create_task(NULL, 1, &argv, 1);

        kfree(argv);

        if (new_task == NULL) {
            printk("task is NULL\n");
        }
        else {
            enqueue_task(new_task);
        }

        index = 0;
        memset(key_buffer, 0, MAX_COMMAND_LENGTH);
        return;
	}
	else if (strcmp(command, "") == 0) {
	}
	else {
		printk("%s: unknown command\n", command);
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
            index--;
			terminal_backspace_cursor(key_buffer[index]);
			key_buffer[index] = '\0';
		}
		return;
	}

	// check if scancode is enter -> execute command
	if (scancode == ENTER) {
		printk("\n");
		shell_exec_command(key_buffer);

		return;
	}

	key_buffer[index++] = scancode;

	putchar(scancode);
}

/* for now, the initialization only prints the prompt */
void shell_init() {
	printk("%s > ", get_current_path());
}

void shell_cleanup(void) {
    index = 0;
    memset(key_buffer, 0, MAX_COMMAND_LENGTH);
    
    shell_init();
}

