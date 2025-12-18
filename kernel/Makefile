CFLAGS:=-MD -O0 -g -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -ffreestanding -Wall -Wextra -Werror -Iinclude -std=gnu99
LDFLAGS:=-Tlinker.ld -nostdlib
ASFLAGS:=-g

KERNEL_SOURCES_C:=$(shell find . -type f -name '*.c')
KERNEL_SOURCES_S:=$(shell find . -type f -name '*.S')
KERNEL_DEPS:=$(shell find . -type f -name '*.d')

#KERNEL_OBJS:=$(KERNEL_SOURCES_C:.c=.o)
#KERNEL_OBJS:=$(KERNEL_OBJS) $(KERNEL_SOURCES_S:.S=.o)
KERNEL_OBJS:=boot.o

OBJS=$(KERNEL_OBJS)

BINARIES=kernel

.PHONY: all clean
.SUFFIXES: .o .c .S

all: $(BINARIES)

kernel: $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

%.o: %.S
	$(AS) $< -o $@ $(ASFLAGS)

clean:
	rm -f $(BINARIES) $(OBJS) *.d */*.d */*/*.d

-include $(KERNEL_DEPS)
