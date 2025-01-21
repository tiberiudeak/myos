#ifndef KERNEL_SHELL_H
#define KERNEL_SHELL_H 1

#include <stdint.h>

#define MAX_COMMAND_LENGTH 256
#define MAX_PARAMS		   100
#define MAX_PARAM_SIZE	   100

#ifdef CONFIG_SH_HISTORY
#define MAX_HISTORY_SIZE CONFIG_SH_HISTORY_MAX_SIZE

struct sh_circular_buffer {
	char commands[MAX_HISTORY_SIZE][MAX_COMMAND_LENGTH];
	int start;
	int count;
};

void history_add_command(const char *);
void history_display(void);
#endif

void shell_scancode(uint8_t);
void shell_init(void);
void shell_cleanup(void);

#endif /* !KERNEL_SHELL_H */
