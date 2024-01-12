#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void main() {
	char* video_memory = (char*) 0xb8000;
	*video_memory = 'A';
}