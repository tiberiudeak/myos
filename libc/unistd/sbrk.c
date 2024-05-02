#include <unistd.h>

void *sbrk(intptr_t increment) {
    uint32_t address;

    __asm__ __volatile__ ("int $0x80" : "=a"(address) : "a"(8), "b"(increment));

    return (void*) address;
}
