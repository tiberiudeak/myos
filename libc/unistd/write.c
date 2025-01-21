#include <unistd.h>

size_t write(int fd, const void *buf, size_t count) {
	size_t written_bytes = -1;

	__asm__ __volatile__("mov %0, %%ebx\n"
						 "mov %1, %%ecx\n"
						 "mov %2, %%esi\n"
						 "int $0x80"
						 :
						 : "r"(fd), "r"(buf), "r"(count), "a"(6)
						 : "ebx", "esi");

	// read return code
	__asm__ __volatile__("mov %%eax, %0" : "=a"(written_bytes));

	return written_bytes;
}
