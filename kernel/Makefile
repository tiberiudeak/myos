# flags
CFLAGS:=--sysroot=$(SYSROOT) -isystem=$(INCLUDEDIR) -O0 -g -fno-stack-protector -include ../$(HEADER_FILE) -ffreestanding -Wall -Wextra -nostdlib
LDFLAGS:=-Tkernel.ld --oformat=binary
ASFLAGS:=

KERNEL_SOURCES_C:=$(shell find . -type f -name '*.c')
KERNEL_SOURCES_S:=$(shell find . -type f -name '*.S')
KERNEL_DEPS:=$(shell find . -type f -name '*.d')

KERNEL_OBJS:=$(KERNEL_SOURCES_C:.c=.o)
KERNEL_OBJS:=$(KERNEL_OBJS) $(KERNEL_SOURCES_S:.S=.o)

OBJS=\
$(KERNEL_OBJS)

LINK_LIST=\
$(LDFLAGS) \
$(KERNEL_OBJS)

BINARIES=kernel

.PHONY: all clean install install-headers install-kernel
.SUFFIXES: .o .c .S

all: $(BINARIES)

kernel: $(OBJS)
	$(LD) $(LINK_LIST) -o $@

%.o: %.c
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS)

%.o: %.S
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -f $(BINARIES) $(OBJS) *.d */*.d */*/*.d

install: install-headers install-kernel

install-headers:
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	cp -R --preserve=timestamps include/. $(DESTDIR)$(INCLUDEDIR)

install-kernel: $(BINARIES)
	mkdir -p ../$(BIN_DIR_SYS)
	cp $(BINARIES) ../$(BIN_DIR_SYS)

-include $(KERNEL_DEPS)
