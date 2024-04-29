#include "include/kernel/shell.h"
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

int nr_params(char *str) {
	int count = 0;
	int i = 0;

	while (str[i] != '\0') {
		if (str[i] == ' ' && str[i + 1] != ' ' &&
			str[i + 1] != '\0') {
			count++;
		}
		i++;
	}

	return count + 1;
}

void tokenize(const char *str, char **argv) {
	int i = 0, j = 0, k = 0;

	while (str[i] != '\0') {
		if (str[i] == ' ') {
			argv[j][k] = '\0';
			j++;
			k = 0;
		}
		else {
			argv[j][k] = str[i];
			k++;
		}
		i++;
	}

	argv[j][k] = '\0';
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
        int number_params = nr_params(command);

        if (number_params > MAX_PARAMS) {
            printk("too many parameters!\n");
        }

        char **argv = kmalloc(sizeof(char*) * number_params);

        if (argv == NULL) {
            return;
        }

        for (int i = 0; i < number_params; i++) {
            argv[i] = kmalloc(sizeof(char) * MAX_PARAM_SIZE);

            if (argv[i] == NULL) {
                // TODO free allocated memory
                return;
            }
        }

        tokenize(command, argv);

        // create new task
#ifdef CONFIG_SIMPLE_SCH
        task_struct *new_task = create_task(execute_elf, 1, &argv, 1);
#else
        task_struct *new_task = create_task(NULL, number_params, argv, 1);
#endif

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

