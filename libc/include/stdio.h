#ifndef _STDIO_H
#define _STDIO_H 1

#define EOF (-1)

#ifdef __cplusplus
extern "C" {
#endif

#define stdin   0
#define stdout  1
#define stderr  2

int printf(const char* __restrict, ...);
int putchar(int);
int puts(const char*);
int putcharc(int, int);
int printfc(int, const char* __restrict, ...);

#ifdef __cplusplus
}
#endif

#endif
