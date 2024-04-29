#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

void cow_print(void) {
    printf("  ---------\n");
    printf("< oh, hello >\n");
    printf("  ---------\n");
    printf("        \\   ^__^\n");
    printf("         \\  (oo)\\_____\n");
    printf("            (__)\\     )\\/\\\n");
    printf("              ||----w |\n");
    printf("              ||     ||\n");
}

void _start(int argc, char *argv[]) {
    printf("argc: %d %x\n", argc, &argc);
    printf("argv[0]: %s\n", argv[0]);
    if (argc >= 2)
        printf("argv[1]: %s\n", &argv[1]);
    cow_print();
    exit(0);
}

