#ifndef _STDIO_H
#define _STDIO_H 1

#define EOF                 (-1)
#define PRINTF_BUFFER_SIZE  250

#ifdef __cplusplus
extern "C" {
#endif

#define stdin   0
#define stdout  1
#define stderr  2

int printf(const char* __restrict, ...);
int puts(const char*);
int fflush(void);
int putchar(int);
int putcharc(int, int);

#ifdef __cplusplus
}
#endif

#endif
