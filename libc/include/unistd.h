#ifndef _UNISTD_H
#define _UNISTD_H 1

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int close(int);
size_t read(int, void*, size_t);
size_t write(int, const void*, size_t);
void *sbrk(intptr_t);

#ifdef __cplusplus
}
#endif
#endif

