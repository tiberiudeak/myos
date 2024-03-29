#include <fcntl.h>

int open(const char *path, int flags) {
    int fd = -1;

    __asm__ __volatile__ ("int $0x80" : "=a"(fd) : "a"(3), "b"(path), "c"(flags));

    return fd;
}

