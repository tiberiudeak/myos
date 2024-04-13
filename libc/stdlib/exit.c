#include <stdlib.h>

void exit(int status) {
    __asm__ __volatile__ ("int $0x80" :: "a"(7), "b"(status));
}

