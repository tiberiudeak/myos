#ifndef KERNEL_IO_H
#define KERNEL_IO_H

#include <stdint.h>

uint8_t port_byte_in(uint16_t port);
void port_byte_out(uint16_t port, uint8_t data);
uint16_t port_word_in(uint16_t port);
void port_word_out(uint16_t port, uint16_t data);
void io_wait(void);

#endif // KERNEL_IO_H
