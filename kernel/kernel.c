#include <stdio.h>
#include <kernel/tty.h>

void test() {
	char* video_memory = (char*) 0xb8000;
	*video_memory = 'A';
	*(video_memory + 2) = 'Y';
}

void kmain() {
	terminal_initialize();
	char *a = "kernel";
	printf("Hello, ");
	printfc(3, "%s", a);
	printf(" World!\n");
}
