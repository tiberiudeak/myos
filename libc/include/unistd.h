#ifndef _UNISTD_H
#define _UNISTD_H 1

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int close(int);
size_t read(int, void*, size_t);

#ifdef __cplusplus
}
#endif
#endif

