#include <tests/tests.h>
#include <fcntl.h>
#include <unistd.h>

/*
 * @brief Perform some tests using the open and close syscalls
 *
 * This function performs tests for open and close syscalls. It uses the file
 * /test.txt in the root directory, so make sure to have that file included in the
 * final os image.
 *
 * @return 1 if error occured, 0 otherwise
 */
uint8_t test_open_close_syscalls(void) {
    int fd1, fd2, fd3, ret;

    fd1 = open("/test.txt", O_RDWR);

    if (fd1 == -1)
        return -1;
    
    fd2 = open("/test.txt", O_RDWR);

    if (fd2 == -1)
        return -1;

    ret = close(fd1);

    if (ret)
        return -1;

    fd3 = open("/test.txt", O_RDWR);

    if (fd3 == -1 || fd3 != fd1)
        return -1;

    ret = close(fd2);

    if (ret)
        return -1;

    ret = close(fd3);

    if (ret)
        return -1;

    fd1 = open("/file_that_does_not_exist", O_RDWR);

    if (fd1 != -1)
        return -1;

    ret = close(fd1);

    if (ret != -1)
        return -1;

    return 0;
}

