HOST?=$(DEFAULT_HOST)
ARCHDIR:=arch/$(HOSTARCH)

# define flags if not already defined
CFLAGS?=-O2 -g
CPPFLAGS?=
LDFLAGS?=-Ttext=0x7c00 --oformat=binary
ASFLAGS?=-I$(ARCHDIR)
LIBS?=

DESTDIR?=
PREFIX?=/usr/local
EXEC_PREFIX?=$(PREFIX)
BOOTDIR?=$(EXEC_PREFIX)/boot
INCLUDEDIR?=$(PREFIX)/include

# add extra flags if needed
CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra
CPPFLAGS:=
LDFLAGS:=$(LDFLAGS)
LIBS:=$(LIBS)

# include arch-specific flags if any
include $(ARCHDIR)/make.config

# final flags containing the default, the extra and
# the arch-specific ones
CFLAGS:=$(CFLAGS) $(BOOT_ARCH_CFLAGS)
CPPFLAGS:=$(CPPFLAGS) $(BOOT_ARCH_CPPFLAGS)
LDFLAGS:=$(LDFLAGS) $(BOOT_ARCH_LDFLAGS)
LIBS:=$(LIBS) $(BOOT_ARCH_LIBS)

BOOT_OBJS:=\
$(BOOT_ARCH_OBJS) \
$(ARCHDIR)/boot.o \
$(ARCHDIR)/2nd_stage_boot.o

OBJS=\
$(BOOT_OBJS)

LINK_LIST=\
$(LDFLAGS) \
$(BOOT_OBJS) \
$(LIBS)

BINARIES=bootloader.bin

.PHONY: all clean install install-headers install-boot
.SUFFIXES: .o .S

all: $(BINARIES)

bootloader.bin: $(OBJS)
	$(LD) $(LINK_LIST) -o $@

.S.o:
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -f $(BINARIES) $(OBJS)

install: install-headers install-boot

# no headers to install for the bootloader
install-headers: ;

install-boot: $(BINARIES)
	mkdir -p $(DESTDIR)$(BOOTDIR)
	cp $(BINARIES) $(DESTDIR)$(BOOTDIR)/.
