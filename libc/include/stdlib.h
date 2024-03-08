#ifndef _STDLIB_H
#define _STDLIB_H 1

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void *malloc(size_t size);
void free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* !_STDLIB_H */
