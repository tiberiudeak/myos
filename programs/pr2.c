#include <stdlib.h>
#include <stdio.h>

void _start(int argc, char *argv[]) {
    void *addr = malloc(0);

    if (addr == NULL) {
        printf("addr if NULL\n");
    }
    else {
        printf("received address: %x\n", addr);
    }

    exit(0);
}

