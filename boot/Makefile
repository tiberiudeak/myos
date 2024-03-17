HOST?=$(DEFAULT_HOST)
ARCHDIR:=arch/$(HOSTARCH)

# define flags if not already defined
CFLAGS?=-O2 -g
CPPFLAGS?=
LDFLAGS?=-Tboot.ld --oformat=binary
ASFLAGS?=-I$(ARCHDIR) --defsym TTY_VBE=1
LIBS?=

# add extra flags if needed
CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra
CPPFLAGS:=
LDFLAGS:=$(LDFLAGS)
LIBS:=$(LIBS)

BOOT_OBJS:=\
$(ARCHDIR)/boot.o \
$(ARCHDIR)/2nd_stage_boot.o \
$(ARCHDIR)/3rd_stage_boot.o

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

%.o: %.S
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -f $(BINARIES) $(OBJS)

install: install-headers install-boot

# no headers to install for the bootloader
install-headers: ;

install-boot: $(BINARIES)
