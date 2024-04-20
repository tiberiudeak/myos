#include <stdlib.h>
#include <stdio.h>
#include <time.h>

void _start(int argc, char *argv[]) {
    printf("this is a test from the ELF file! %x\n", &argv);
    while(1);
    sleep(10000);
    int *test = (int*)0xC0000000;
    *test = 1;

    exit(1);
}

