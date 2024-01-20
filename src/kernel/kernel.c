#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

void test() {
	char* video_memory = (char*) 0xb8000;
	*video_memory = 'A';
	*(video_memory + 2) = 'Y';
}

void kmain() {
	char* video_memory = (char*) 0xb8000;
	*video_memory = 'A';
	//test();
	int a = strlen("Hello, world!");
}
