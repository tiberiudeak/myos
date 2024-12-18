#include <stdlib.h>
#include <stdio.h>
#include <time.h>

void _start(int argc, char *argv[]) {
    printf("running an infinite loop...\n");
    while(1) {
		printf("z...");
		sleep(1000);
	}

    exit(0);
}

