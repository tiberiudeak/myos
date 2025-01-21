#ifndef _KSTRING_H
#define _KSTRING_H 1

#include <stddef.h>

int memcmp(const void*, const void*, size_t);
void* memcpy(void* __restrict, const void* __restrict, size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);
size_t strlen(const char*);
char* strcpy(char*, const char*);
char* strncpy(char*, const char*, size_t);
char* strcat(char*, const char*);
char* strncat(char*, const char*, size_t);
int strcmp(const char*, const char*);
int strncmp(const char*, const char*, size_t);
char* strchr(const char*, int);
char* strrchr(const char*, int);
char* strstr(const char*, const char*);
char* strrstr(const char*, const char*);
char* itoa(int, char*, int);
void reverse(char*);
int atoi(char *, int *);
char *ftoa(float,  char*, int);

#endif // _KSTRING_H
