#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void usage(void) {
    printf("usage: ./calculator <operation> <op1> <op2>\n");
    printf("\t<operation>: + - * /\n");
    printf("\t example: ./calculator + 1 2\n");
}

void _start(int argc, char *argv[]) {
    int error;

    if (argc != 4) {
        usage();
        exit(1);
    }

    char *operation = argv[1];

    int op1 = atoi(argv[2], &error);

    if (error) {
        printf("invalid op1\n");
        exit(1);
    }

    int op2 = atoi(argv[3], &error);

    if (error) {
        printf("invalid op2\n");
        exit(1);
    }

    if (strcmp(operation, "+") == 0) {
        printf("%d\n", op1 + op2);
    }
    else if (strcmp(operation, "-") == 0) {
        printf("%d\n", op1 - op2);
    }
    else if (strcmp(operation, "*") == 0) {
        printf("%d\n", op1 * op2);
    }
    else if (strcmp(operation, "/") == 0) {
        if (op2 == 0) {
            printf("error\n");
        }
        else {
            printf("%d\n", op1 / op2);
        }
    }
    else {
        printf("invalid operation\n");
    }

    exit(0);
}

