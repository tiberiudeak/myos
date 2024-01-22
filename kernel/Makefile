HOST?=$(DEFAULT_HOST)
ARCHDIR:=arch/$(HOSTARCH)

# define flags if not already defined
CFLAGS?=-O2 -g
CPPFLAGS?=
LDFLAGS?=-Ttext=0x8000 --oformat=binary
ASFLAGS?=
LIBS?=

DESTDIR?=
PREFIX?=/usr/local
EXEC_PREFIX?=$(PREFIX)
KERNELDIR?=$(EXEC_PREFIX)/kernel
INCLUDEDIR?=$(PREFIX)/include

# add extra flags if needed
CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra
CPPFLAGS:=$(CPPFLAGS)
LDFLAGS:=$(LDFLAGS) -L../libc
LIBS:=$(LIBS) -nostdlib -lk

# include arch-specific flags if any
include $(ARCHDIR)/make.config

# final flags containing the default, the extra and
# the arch-specific ones
CFLAGS:=$(CFLAGS) $(KERNEL_ARCH_CFLAGS)
CPPFLAGS:=$(CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)
LDFLAGS:=$(LDFLAGS) $(KERNEL_ARCH_LDFLAGS)
LIBS:=$(LIBS) $(KERNEL_ARCH_LIBS)

KERNEL_OBJS:=\
$(KERNEL_ARCH_OBJS) \
arch/i386/kernel_entry.o \
kernel.o \
arch/i386/tty.o \

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

.c.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS) $(CPPFLAGS)

.S.o:
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -f $(BINARIES) $(OBJS) *.d */*.d */*/*.d

install: install-headers install-kernel

install-headers: ;

install-kernel: $(BINARIES)
	mkdir -p $(DESTDIR)$(KERNELDIR)
	cp $(BINARIES) $(DESTDIR)$(KERNELDIR)/.
