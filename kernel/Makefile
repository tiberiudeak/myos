HOST?=$(DEFAULT_HOST)
ARCHDIR:=arch/$(HOSTARCH)

# define flags if not already defined
CFLAGS?=-O2 -g
CPPFLAGS?=
LDFLAGS?=-Tkernel.ld --oformat=binary
ASFLAGS?=
LIBS?=

# add extra flags if needed
CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra
CPPFLAGS:=$(CPPFLAGS) -D__is_kernel -I.
LDFLAGS:=$(LDFLAGS) -L../libc
LIBS:=$(LIBS) -nostdlib -lk

KERNEL_SOURCES_C:=$(shell find . -type f -name '*.c')
KERNEL_SOURCES_S:=$(shell find . -type f -name '*.S')
KERNEL_DEPS:=$(shell find . -type f -name '*.d')

KERNEL_OBJS:=$(KERNEL_SOURCES_C:.c=.o)
KERNEL_OBJS:=$(KERNEL_OBJS) $(KERNEL_SOURCES_S:.S=.o)

OBJS=\
$(KERNEL_OBJS)

LINK_LIST=\
$(LDFLAGS) \
$(KERNEL_OBJS) \
$(LIBS)

BINARIES=kernel.bin

.PHONY: all clean install install-headers install-kernel
.SUFFIXES: .o .c .S

all: $(BINARIES)

kernel.bin: $(OBJS)
	$(LD) $(LINK_LIST) -o $@

%.o: %.c
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS) $(CPPFLAGS)

%.o: %.S
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -f $(BINARIES) $(OBJS) *.d */*.d */*/*.d

install: install-headers install-kernel

install-headers:
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	cp -R --preserve=timestamps include/. $(DESTDIR)$(INCLUDEDIR)/.

install-kernel: $(BINARIES)

-include $(KERNEL_DEPS)
