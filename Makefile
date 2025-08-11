export PROJECTS=libc kernel programs

export HOST:=i686-elf

export MAKE:=make
export AR:=$(HOST)-ar
export AS:=$(HOST)-as
export CC:=$(HOST)-gcc
export LD:=$(HOST)-ld

export HEADER_FILE:=config.h

KERNEL_SRC_DIR:=kernel
LIBC_SRC_DIR:=libc
PROG_SRC_DIR:=programs

KERNEL_BIN:=$(KERNEL_SRC_DIR)/kernel
LIBC_AR:=$(LIBC_SRC_DIR)/libc.a

BINARIES:=$(KERNEL_BIN)
ISODIR:=isodir
ISO:=myos.iso

QEMU:=qemu-system-i386
QEMUFLAGS:=-kernel $(KERNEL_BIN) -rtc base=localtime,clock=host,driftfix=none
QEMUFLAGS_DEBUG:=-kernel $(KERNEL_BIN) -rtc base=localtime,clock=host,driftfix=none -S -s
QEMUFLAGS_ISO:=-cdrom $(ISO) -rtc base=localtime,clock=host,driftfix=none

.PHONY: all clean run iso runiso kernel userspace

all: $(KERNEL_BIN)

userspace: $(LIBC_AR)
	@$(MAKE) -C $(PROG_SRC_DIR)

kernel: $(BINARIES)

$(KERNEL_BIN): $(HEADER_FILE)
	@$(MAKE) -C $(KERNEL_SRC_DIR)

$(LIBC_AR):
	@$(MAKE) -C $(LIBC_SRC_DIR)

$(HEADER_FILE): .config
	@./generate_config_header.sh

.config: my_ncurses_menu.c my_ncurses_menu.h
	gcc -o menu my_ncurses_menu.c -lncurses
	./menu

run: $(KERNEL_BIN)
	$(QEMU) $(QEMUFLAGS)

gdb-debug: $(KERNEL_BIN)
	$(QEMU) $(QEMUFLAGS_DEBUG)

runiso: $(ISO)
	$(QEMU) $(QEMUFLAGS_ISO)

iso: $(ISO)

$(ISO): $(KERNEL_BIN) grub.cfg
	@mkdir -p $(ISODIR)/boot/grub
	@cp $(KERNEL_BIN) $(ISODIR)/boot/myos.bin
	@cp grub.cfg $(ISODIR)/boot/grub/grub.cfg
	@grub-mkrescue -o myos.iso $(ISODIR)

clean:
	@for PROJECT in $(PROJECTS); do \
		$(MAKE) -C $$PROJECT clean; \
	done

	rm -rf $(HEADER_FILE) $(ISO) $(ISODIR) menu
