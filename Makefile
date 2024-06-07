export SYSTEM_HEADER_PROJECTS=boot libc kernel programs
export PROJECTS=boot libc kernel programs

export HOST:=i686-elf
export DEFAULT_HOST:=i686-elf

export HOSTARCH:=
# check if the HOST contains the string i[digit]86
# and set the HOSTARCH to i386 if it does
# TODO: otherwise, set it to the arch in the HOST name
ifeq ($(findstring i[0-9]86,$(HOST)),)
	HOSTARCH+=i386
endif

export HEADER_FILE := config.h

export MAKE:=make
export AR:=$(HOST)-ar
export AS:=$(HOST)-as
export CC:=$(HOST)-gcc
export LD:=$(HOST)-ld

export CFLAGS:=-O0 -g -fno-stack-protector -include ../$(HEADER_FILE)
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
QEMUFLAGS_DEBUG:= -drive format=raw,file=myos.bin,if=ide,index=0,media=disk -rtc base=localtime,clock=host,driftfix=none -S -s

# isystem=<directory> means that the compiler will look
# for system headers in <directory>
CC:=$(CC) -isystem=$(INCLUDEDIR)

# directories containing the source code for the projects
BOOT_SRC_DIR:=boot
KERNEL_SRC_DIR:=kernel
LIBC_SRC_DIR:=libc
PROG_SRC_DIR:=programs
export PROG_BIN_DIR:=bin

# final paths to the binaries
BOOT_BIN:=$(BOOT_SRC_DIR)/bootloader
KERNEL_BIN:=$(KERNEL_SRC_DIR)/kernel
LIBC_AR:=$(LIBC_SRC_DIR)/libc.a

# list of all the binaries
BINARIES:=\
$(BOOT_BIN) \
$(KERNEL_BIN)

TARGET:=myos.bin
INCLUDED_FILES:=files.txt

.PHONY: all kernel clean run menuconfig

all: $(TARGET)

$(TARGET): $(BINARIES)
	# compile programs in the programs folder
	@$(MAKE) -C $(PROG_SRC_DIR) install

	gcc create_disk_image.c -o create_disk_image -lm
	./create_disk_image $(TARGET)

$(BOOT_BIN): $(HEADER_FILE)
	@rm -rf $(PROG_BIN_DIR)
	@mkdir -p $(SYSROOT)

	@$(MAKE) -C $(BOOT_SRC_DIR) install

$(KERNEL_BIN): $(LIBC_AR) $(HEADER_FILE)
	@mkdir -p $(SYSROOT)

	@$(MAKE) -C $(KERNEL_SRC_DIR) install

$(LIBC_AR):
	@mkdir -p $(SYSROOT)

	@$(MAKE) -C $(LIBC_SRC_DIR) install

$(HEADER_FILE): .config
	./generate_config_header.sh

.config: my_ncurses_menu.c my_ncurses_menu.h
	gcc -o menu my_ncurses_menu.c -lncurses
	./menu

run: $(TARGET)
	$(QEMU) $(QEMUFLAGS)

gdb-debug: $(TARGET)
	$(QEMU) $(QEMUFLAGS_DEBUG)

menuconfig: my_ncurses_menu.c my_ncurses_menu.h
	gcc -o menu my_ncurses_menu.c -lncurses
	./menu

menuconfig-steps: my_ncurses_menu.c my_ncurses_menu.h
	gcc -DSTEP_BY_STEP -o menu my_ncurses_menu.c -lncurses
	./menu

# default configuration profiles - each profile must have an entry here
default: profiles/default
	cp $^ ./.config
	make run

minimal: profiles/minimal
	cp $^ ./.config
	make run

performance: profiles/performance
	cp $^ ./.config
	make run

debug: profiles/debug
	cp $^ ./.config
	make run

clean:
	@for PROJECT in $(PROJECTS); do \
		$(MAKE) -C $$PROJECT clean; \
	done

	rm -rf $(TARGET) create_disk_image $(HEADER_FILE) menu $(PROG_BIN_DIR)
