export SYSTEM_HEADER_PROJECTS=boot libc kernel
export PROJECTS=boot libc kernel

export HOST:=i686-elf
export DEFAULT_HOST:=i686-elf

export HOSTARCH:=
# check if the HOST contains the string i[digit]86
# and set the HOSTARCH to i386 if it does
# TODO: otherwise, set it to the arch in the HOST name
ifeq ($(findstring i[0-9]86,$(HOST)),)
	HOSTARCH+=i386
endif

export MAKE:=make
export AR:=$(HOST)-ar
export AS:=$(HOST)-as
export CC:=$(HOST)-gcc
export LD:=$(HOST)-ld

export CFLAGS:=-O0 -g -fno-stack-protector
export CPPFLAGS:=

export SYSROOT:=$(shell pwd)/sysroot
export DESTDIR:=$(SYSROOT)
export CC:=$(CC) --sysroot=$(SYSROOT)

export PREFIX:=/usr
export BOOTDIR:=/boot
export INCLUDEDIR:=$(PREFIX)/include
export LIBDIR:=$(PREFIX)/lib
export KERNELDIR:=/kernel

QEMU:=qemu-system-i386
QEMUFLAGS:=

# check if the HOST contains the string `elf` and add
# the include path to the compiler using the -isystem flag
ifneq ($(findstring -elf,$(HOST)),)
	CC:=$(CC) -isystem=$(INCLUDEDIR)
endif

# directories containing the source code for the projects
BOOT_SRC_DIR:=boot
KERNEL_SRC_DIR:=kernel
LIBC_SRC_DIR:=libc

# final paths to the binaries (relative to the sysroot)
BOOT_BIN:=$(SYSROOT)$(BOOTDIR)/bootloader.bin
KERNEL_BIN:=$(SYSROOT)$(KERNELDIR)/kernel.bin
LIBC_AR:=$(SYSROOT)$(LIBDIR)/libc.a

# list of all the binaries
BINARIES:=\
$(BOOT_BIN) \
$(KERNEL_BIN)

TARGET:=myos.bin

.PHONY: all kernel clean run

all: $(TARGET)

$(TARGET): $(BINARIES)
	cat $^ > $@

$(BOOT_BIN):
	@mkdir -p $(SYSROOT)

	@$(MAKE) -C $(BOOT_SRC_DIR) install

$(KERNEL_BIN): $(LIBC_AR)
	@mkdir -p $(SYSROOT)

	@$(MAKE) -C $(KERNEL_SRC_DIR) install

$(LIBC_AR):
	@mkdir -p $(SYSROOT)

	@$(MAKE) -C $(LIBC_SRC_DIR) install

run: $(TARGET)
	$(QEMU) $(QEMUFLAGS) $<

clean:
	@for PROJECT in $(PROJECTS); do \
		$(MAKE) -C $$PROJECT clean; \
	done

	rm -rf $(SYSROOT) $(TARGET)
