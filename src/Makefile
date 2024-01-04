
BINARY=boot_sector.bin os.bin
NASM=nasm

REM=$(foreach b,$(BINARY),$(b))

all: $(BINARY)

boot_sector.bin: myboot.asm
	$(NASM) $< -f bin -o boot_sector.bin

os.bin: boot_sector.bin
	cat $^ > os.bin

run: os.bin
	qemu-system-x86_64 $<

.PHONY: clean

clean:
	rm -rf $(REM)