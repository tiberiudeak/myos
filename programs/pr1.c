#include <stdlib.h>
#include <stdio.h>

void _start(int argc, char *argv[]) {
    __asm__ __volatile__ ("int $0x80"::"a"(1), "b"(1));
    __asm__ __volatile__ ("int $0x80"::"a"(2));
    printf("this is a test from the ELF file!\n");

    exit(0);
}

