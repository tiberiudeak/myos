#ifndef _KUTILS_H
#define _KUTILS_H 1

#define stdin   0
#define stdout  1
#define stderr  2

typedef enum {
    O_RDONLY            = 0x1,  // open file only with read permissions
    O_WRONLY            = 0x2,  // open file only with write permissions
    O_RDWR              = 0x4,  // open file with read-write permissions
    O_CREAT             = 0x8   // create file if it doesn't exits
} OPEN_FLAGS;

int ceil(int a, int b);

#endif
