
DEFAULT_HOST!=../default-host.sh
HOST?=$(DEFAULT_HOST)
HOSTARCH!=../target-triplet-to-arch.sh $(HOST)

ARCHDIR:=arch/$(HOSTARCH)

CFLAGS?=-O2 -g
CPPFLAGS?=
LDFLAGS?=-Ttext=0x0000 --oformat=binary
ASFLAGS?=
LIBS?=

# DESTDIT set when running make install (it will be the sysroot)
DESTDIR?=
PREFIX?=/usr/local
EXEC_PREFIX?=$(PREFIX)
KERNELDIR?=$(EXEC_PREFIX)/kernel
INCLUDEDIR?=$(PREFIX)/include

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra
CPPFLAGS:=$(CPPFLAGS)
LDFLAGS:=$(LDFLAGS)
LIBS:=$(LIBS) -nostdlib

# include arch-specific flags if any
include $(ARCHDIR)/make.config

CFLAGS:=$(CFLAGS) $(KERNEL_ARCH_CFLAGS)
CPPFLAGS:=$(CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)
LDFLAGS:=$(LDFLAGS) $(KERNEL_ARCH_LDFLAGS)
LIBS:=$(LIBS) $(KERNEL_ARCH_LIBS)

KERNEL_OBJS:=\
$(KERNEL_ARCH_OBJS) \
kernel.o

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
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

clean:
	rm -f $(BINARIES) $(OBJS) *.d

install: install-headers install-kernel

install-headers: ;

install-kernel: $(BINARIES)
	mkdir -p $(DESTDIR)$(KERNELDIR)
	cp $(BINARIES) $(DESTDIR)$(KERNELDIR)/.
