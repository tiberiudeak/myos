#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

void cow_print(char *msg) {
    int len = strlen(msg);

    printf("  ");
    for (int i = 0; i < len; i++)
        printf("-");
    printf(" \n");

    printf("< %s >\n", msg);

    printf("  ");
    for (int i = 0; i < len; i++)
        printf("-");
    printf(" \n");

    printf("        \\   ^__^\n");
    printf("         \\  (oo)\\_____\n");
    printf("            (__)\\     )\\/\\\n");
    printf("              ||----w |\n");
    printf("              ||     ||\n");
}

void _start(int argc, char *argv[]) {
    // printf("argc: %d\n", argc);
    // printf("argv[0]: %s\n", argv[0]);
    if (argc >= 2)
        printf("argv[1]: %s\n", argv[1]);

    if (argc >= 2)
        cow_print(argv[1]);
    else {
        cow_print("oh, hello!");
    }

    exit(0);
}

