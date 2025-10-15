CFLAGS:=-O0 -g -ffreestanding -Wall -Wextra -Iinclude -std=gnu99 -include ../config.h
LDFLAGS:=-Tlinker.ld -nostdlib
ASFLAGS:=-g

KERNEL_SOURCES_C:=$(shell find . -type f -name '*.c')
KERNEL_SOURCES_S:=$(shell find . -type f -name '*.S')
KERNEL_DEPS:=$(shell find . -type f -name '*.d')

KERNEL_OBJS:=$(KERNEL_SOURCES_C:.c=.o)
KERNEL_OBJS:=$(KERNEL_OBJS) $(KERNEL_SOURCES_S:.S=.o)

OBJS=$(KERNEL_OBJS)

BINARIES=kernel

.PHONY: all clean
.SUFFIXES: .o .c .S

all: $(BINARIES)

kernel: $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) -MD -c $< -o $@ $(CFLAGS)

%.o: %.S
	$(AS) $< -o $@ $(ASFLAGS)

clean:
	rm -f $(BINARIES) $(OBJS) *.d */*.d */*/*.d

-include $(KERNEL_DEPS)
