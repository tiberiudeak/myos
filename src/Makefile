
BINARY=boot_sector.o os.bin

AS=i686-elf-as
GCC=i686-elf-gcc
LD=i686-elf-ld

REM=$(foreach b,$(BINARY),$(b))

all: $(BINARY)

boot_sector.o: myboot.S
	$(AS) $< -o $@

os.bin: boot_sector.o
	$(LD) -Ttext 0x7c00 --oformat binary $< -o $@

run: os.bin
	qemu-system-x86_64 $<

.PHONY: clean

clean:
	rm -rf $(REM)