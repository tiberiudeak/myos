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
    int i = 1;
    char buf[10];
    strncpy(buf, "ola?", 5);

    printf("test1 %d\na", i);
    printf("test2%s", buf);
    printf("bb\n");
    printf("c");
    cow_print();
    exit(0);
}

