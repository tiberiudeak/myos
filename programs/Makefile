CFLAGS:=-O0 -g -Wall -fPIE -I../libc/include -ffreestanding -std=gnu99
LDFLAGS:=-nostdlib -lk -L../libc

EXEC_SOURCES_C:=$(shell find . -type f -name '*.c')
KERNEL_DEPS:=$(shell find . -type f -name '*.d')

EXEC_OBJS:=$(EXEC_SOURCES_C:.c=.o)

OBJS=$(EXEC_OBJS)

BINARIES:=$(foreach obj,$(OBJS),$(basename $(obj)))

.PHONY: all clean
.SUFFIXES: .o .c

all: $(BINARIES)

$(BINARIES): %: %.o
	$(LD) $< -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) -MD -c $< -o $@ $(CFLAGS)

clean:
	rm -rf $(BINARIES) $(OBJS) *.d */*.d */*/*.d
