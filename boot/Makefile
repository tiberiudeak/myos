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

# check if CONFIG_TTY_VBE is enabled and if so add it as a symbol
# TODO: find a better solution, for now there are only three configurations that
# affects the bootloader, but if there will be more in the future ...
TTY_VBE_CONF := $(shell grep -q "^#define CONFIG_TTY_VBE" ../$(HEADER_FILE) && echo 1 || echo 0)

ifeq ($(TTY_VBE_CONF),1)
	# get screen resolution from config.h
	TTY_VBE_WIDTH=$(shell grep "CONFIG_TTY_VBE_WIDTH" ../$(HEADER_FILE) | cut -d ' ' -f3)
	TTY_VBE_HEIGHT=$(shell grep "CONFIG_TTY_VBE_HEIGHT" ../$(HEADER_FILE) | cut -d ' ' -f3)

	ASFLAGS += --defsym CONFIG_TTY_VBE=1 --defsym CONFIG_TTY_VBE_WIDTH=$(TTY_VBE_WIDTH) --defsym CONFIG_TTY_VBE_HEIGHT=$(TTY_VBE_HEIGHT)
endif

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

%.o: %.S ../$(HEADER_FILE)
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -f $(BINARIES) $(OBJS)

install: install-headers install-boot

# no headers to install for the bootloader
install-headers: ;

install-boot: $(BINARIES)
	cp $(BINARIES) ../bin

