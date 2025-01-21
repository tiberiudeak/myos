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

# separate sysroots for kernel/bootloader and for
# userspace programs
export SYSROOT:=$(shell pwd)/sysroot
export SYSROOT_USER:=$(shell pwd)/sysroot_user
export DESTDIR:=$(SYSROOT)
export DESTDIR_USER:=$(SYSROOT_USER)

# folder in sysroot with header filed
export INCLUDEDIR:=/include

QEMU:=qemu-system-i386
QEMUFLAGS:= -drive format=raw,file=myos.bin,if=ide,index=0,media=disk -rtc base=localtime,clock=host,driftfix=none
QEMUFLAGS_DEBUG:= -drive format=raw,file=myos.bin,if=ide,index=0,media=disk -rtc base=localtime,clock=host,driftfix=none -S -s

# directories containing the source code for the projects
BOOT_SRC_DIR:=boot
KERNEL_SRC_DIR:=kernel
LIBC_SRC_DIR:=libc
PROG_SRC_DIR:=programs

# final paths to the binaries
BOOT_BIN:=$(BOOT_SRC_DIR)/bootloader
KERNEL_BIN:=$(KERNEL_SRC_DIR)/kernel
LIBC_AR:=$(LIBC_SRC_DIR)/libc.a

# separate binaries into system binaries (kernel and
# bootloader (still)) and userspace binaries (programs
# compiled with the provided libc that will be present
# in the filesystem)
export BIN_DIR_SYS:=bin_system
export BIN_DIR_USER:=bin_user
BIN_DIR_GLOBAL:=bin

# list of all the binaries
BINARIES:=\
$(BOOT_BIN) \
$(KERNEL_BIN)

TARGET:=myos.bin
INCLUDED_FILES:=files.txt

.PHONY: all kernel clean run menuconfig userspace

all: $(TARGET)

# target to build the os image including
# kernel is mandatory, userspace not - if you want
# to include the userspace alco, compile it with
# make userspace
$(TARGET): kernel
	@rm -rf $(BIN_DIR_GLOBAL)
	@mkdir -p $(BIN_DIR_GLOBAL)
	@cp $(BIN_DIR_SYS)/* $(BIN_DIR_GLOBAL)

# add userspace if present
ifneq ($(wildcard $(BIN_DIR_USER)),)
	cp $(BIN_DIR_USER)/* $(BIN_DIR_GLOBAL)
endif

	@gcc create_disk_image.c -o create_disk_image -lm
	@./create_disk_image $(TARGET)

# compile only the userspace - including the provided libc
# and all programs in the programs folder
userspace: $(LIBC_AR)
	@$(MAKE) -C $(PROG_SRC_DIR) install

# compile kernel (and also bootloader for now)
kernel: $(BINARIES)

# compile bootloader
$(BOOT_BIN): $(HEADER_FILE)
	@mkdir -p $(SYSROOT)

	@$(MAKE) -C $(BOOT_SRC_DIR) install

# compile kernel
$(KERNEL_BIN): $(HEADER_FILE)
	@mkdir -p $(SYSROOT)

	@$(MAKE) -C $(KERNEL_SRC_DIR) install

# compile libc
$(LIBC_AR):
	@mkdir -p $(SYSROOT)

	@$(MAKE) -C $(LIBC_SRC_DIR) install

# generate config header file
$(HEADER_FILE): .config
	@./generate_config_header.sh

.config: my_ncurses_menu.c my_ncurses_menu.h
	gcc -o menu my_ncurses_menu.c -lncurses
	./menu

# run in normal mode
run: $(TARGET)
	$(QEMU) $(QEMUFLAGS)

# run in debug mode
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
	@echo "Getting the default configuration profile..."
	@cp $^ ./.config
	@make

minimal: profiles/minimal
	@echo "Getting the minimal configuration profile..."
	@cp $^ ./.config
	@make

performance: profiles/performance
	@echo "Getting the performance configuration profile..."
	@cp $^ ./.config
	@make

debug: profiles/debug
	@echo "Getting the debug configuration profile..."
	@cp $^ ./.config
	@make

clean:
	@for PROJECT in $(PROJECTS); do \
		$(MAKE) -C $$PROJECT clean; \
	done

	rm -rf $(TARGET) create_disk_image $(HEADER_FILE) menu $(BIN_DIR_USER) $(BIN_DIR_SYS) $(BIN_DIR_GLOBAL)
