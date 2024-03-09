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

# set system root directory for the compiler
# sysroot=<directory> means that the compiler will look
# for libraries and headers in <directory>
export CC:=$(CC) --sysroot=$(SYSROOT)

export PREFIX:=/usr
export INCLUDEDIR:=$(PREFIX)/include
export LIBDIR:=$(PREFIX)/lib

QEMU:=qemu-system-i386
QEMUFLAGS:= -drive format=raw,file=myos.bin,if=ide,index=0,media=disk -rtc base=localtime,clock=host,driftfix=none

# isystem=<directory> means that the compiler will look
# for system headers in <directory>
CC:=$(CC) -isystem=$(INCLUDEDIR)

# directories containing the source code for the projects
BOOT_SRC_DIR:=boot
KERNEL_SRC_DIR:=kernel
LIBC_SRC_DIR:=libc

# final paths to the binaries
BOOT_BIN:=$(BOOT_SRC_DIR)/bootloader.bin
KERNEL_BIN:=$(KERNEL_SRC_DIR)/kernel.bin
LIBC_AR:=$(LIBC_SRC_DIR)/libc.a

# list of all the binaries
BINARIES:=\
$(BOOT_BIN) \
$(KERNEL_BIN)

TARGET:=myos.bin

.PHONY: all kernel clean run

all: $(TARGET)

$(TARGET): $(BINARIES)
	gcc create_disk_image.c -o create_disk_image
	./create_disk_image $(TARGET)

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
	$(QEMU) $(QEMUFLAGS)

clean:
	@for PROJECT in $(PROJECTS); do \
		$(MAKE) -C $$PROJECT clean; \
	done

	rm -rf $(TARGET) create_disk_image
