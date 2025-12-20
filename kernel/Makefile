CFLAGS:=-MD -O0 -g -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -ffreestanding -Wall -Wextra -Werror -Iinclude -m64
LDFLAGS:=-Tlinker.ld -nostdlib -z max-page-size=0x1000
ASFLAGS:=-g

KERNEL_SOURCES_C:=$(shell find . -type f -name '*.c')
KERNEL_SOURCES_S:=$(shell find . -type f -name '*.S')
KERNEL_DEPS:=$(shell find . -type f -name '*.d')

#KERNEL_OBJS:=$(KERNEL_SOURCES_C:.c=.o)
#KERNEL_OBJS:=$(KERNEL_OBJS) $(KERNEL_SOURCES_S:.S=.o)
KERNEL_OBJS:=boot32.o boot64.o main.o

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
