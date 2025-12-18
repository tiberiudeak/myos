export PROJECTS=kernel

export HOST:=x86_64-elf

export MAKE:=make
export AR:=$(HOST)-ar
export AS:=$(HOST)-as
export CC:=$(HOST)-gcc
export LD:=$(HOST)-ld

KERNEL_SRC_DIR:=kernel

KERNEL_BIN:=$(KERNEL_SRC_DIR)/kernel

BINARIES:=$(KERNEL_BIN)
ISODIR:=isodir
ISO:=myos.iso

QEMU:=qemu-system-x86_64
QEMUFLAGS:=-cdrom $(ISO) -rtc base=localtime,clock=host,driftfix=none
QEMUFLAGS_DEBUG:=-cdrom $(ISO) -rtc base=localtime,clock=host,driftfix=none -S -s

.PHONY: all clean run iso kernel

all: $(KERNEL_BIN)

kernel: $(BINARIES)

$(KERNEL_BIN):
	@$(MAKE) -C $(KERNEL_SRC_DIR)

run: $(KERNEL_BIN) $(ISO)
	$(QEMU) $(QEMUFLAGS)

gdb-debug: $(KERNEL_BIN) $(ISO)
	$(QEMU) $(QEMUFLAGS_DEBUG)

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

	rm -rf $(ISO) $(ISODIR)
