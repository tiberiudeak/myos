#ifndef _FCNTL_H
#define _FCNTL_H 1

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	O_RDONLY = 0x1, // open file only with read permissions
	O_WRONLY = 0x2, // open file only with write permissions
	O_RDWR = 0x4,	// open file with read-write permissions
	O_CREAT = 0x8	// create file if it doesn't exits
} OPEN_FLAGS;

int open(const char *pathname, int flags);

#ifdef __cplusplus
}
#endif

#endif
