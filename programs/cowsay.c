#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    cow_print();
    exit(0);
}

