
BINARY=boot_sector.bin kernel.bin file_table.bin os.bin
NASM=nasm

REM=$(foreach b,$(BINARY),../bin/$(b))

all: $(BINARY)

boot_sector.bin: boot_sector2.asm
	$(NASM) $< -f bin -o boot_sector.bin

kernel.bin: kernel.asm
	$(NASM) $< -f bin -o kernel.bin

file_table.bin: file_table.asm
	$(NASM) $< -f bin -o file_table.bin

os.bin: boot_sector.bin file_table.bin kernel.bin
	cat $^ > os.bin
	mv *.bin ../bin

run: ../bin/os.bin
	qemu-system-x86_64 -nographic $<

.PHONY: clean

clean:
	rm -rf $(REM)