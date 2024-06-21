#include <stdlib.h>
#include <stdio.h>

void _start(int argc, char *argv[]) {
    void *addr = malloc(12);

    if (addr == NULL) {
        printf("addr is NULL\n");
    }
    else {
        printf("received address: %x\n", addr);
    }

    void *addr2 = malloc(12);

    if (addr2 == NULL) {
        printf("addr is NULL\n");
    }
    else {
        printf("received address: %x\n", addr2);
    }

    void *addr3 = malloc(4016);

    if (addr3 == NULL) {
        printf("addr is NULL\n");
    }
    else {
        printf("received address: %x\n", addr3);
    }

    void *addr4 = malloc(9084);

    if (addr4 == NULL) {
        printf("addr is NULL\n");
    }
    else {
        printf("received address: %x\n", addr4);
    }
    //free(addr);
    void *addr5 = malloc(1);

    if (addr5 == NULL) {
        printf("addr is NULL\n");
    }
    else {
        printf("received address: %x\n", addr5);
    }

    free(addr);
    free(addr2);
    free(addr3);
    free(addr4);
    free(addr5);

    exit(0);
}

