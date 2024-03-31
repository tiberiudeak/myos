#include <unistd.h>

/**
 * @brief Read count bytes from the file given through the file descriptor into buf
 *
 * Read count bytes from the file represented by the given file descriptor and store
 * the read bytes at address buf. The arguments are put into EAX, EBX, ECX and ESI
 * in this order.
 *
 * @param   fd      The file descriptor
 * @param   buf     The location where to store the bytes read
 * @param   count   Number of bytes to read
 *
 * @return Number of bytes read, 0 if end of file, or -1 if error
 */
size_t read(int fd, void *buf, size_t count) {

    size_t bytes_read = -1;

    __asm__ __volatile__ (
            "mov %0, %%ebx\n"
            "mov %1, %%ecx\n"
            "mov %2, %%esi\n"
            "int $0x80" : : "r"(fd), "r"(buf), "r"(count), "a"(5):
            "ebx", "esi");

    // read return code
    __asm__ __volatile__ ("mov %%eax, %0" : "=a"(bytes_read));

    return bytes_read;
}

