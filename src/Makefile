
BINARY=boot_sector.bin kernel.bin os.bin
NASM=nasm

REM=$(foreach b,$(BINARY),../bin/$(b))

all: $(BINARY)

boot_sector.bin: boot_sector2.asm
	$(NASM) $< -f bin -o boot_sector.bin

kernel.bin: kernel.asm
	$(NASM) $< -f bin -o kernel.bin

os.bin: boot_sector.bin kernel.bin
	cat $^ > os.bin
	mv *.bin ../bin

run: ../bin/boot_sector.bin
	qemu-system-x86_64 $<

.PHONY: clean

clean:
	rm -rf $(REM)