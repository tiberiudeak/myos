#include <unistd.h>

int close(int fd) {
	int ret;

	__asm__ __volatile__("int $0x80" : "=a"(ret) : "a"(4), "b"(fd));

	return ret;
}
