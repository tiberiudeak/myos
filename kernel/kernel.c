#include <kernel/tty.h>
#include <kernel/acpi.h>
#include <kernel/keyboard.h>
#include <kernel/shell.h>
#include <arch/i386/gdt.h>
#include <arch/i386/idt.h>
#include <arch/i386/ps2.h>
#include <arch/i386/pit.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/kmalloc.h>
#include <stdio.h>
#include <fs.h>

extern char kernel_end[];

void halt_processor(void) {
	__asm__ __volatile__ ("cli; hlt");
}

void print_char_vbe(uint32_t* framebuffer, int width, int height, int x, int y, char c, uint8_t* font) {
    // Calculate the offset within the font data
    int font_offset = c * 16;

    // Calculate the starting address of the character's pixel data in the font
    uint8_t* char_data = font + font_offset;

    // Calculate the starting address of the character's position in the framebuffer
    uint32_t* dest = framebuffer + (y * width + x);

    // Iterate over each row of the character
    for (int row = 0; row < 16; row++) {
        uint8_t pixel_data = char_data[row]; // Get the pixel data for this row
        uint32_t* dest_row = dest + row * width; // Calculate the starting address of this row in the framebuffer

        // Iterate over each pixel in the row
        for (int col = 0; col < 8; col++) {
            // Check if the current pixel is set in the pixel data
            if (pixel_data & (1 << (7 - col))) {
                // Set the corresponding pixel in the framebuffer
                dest_row[col] = 0xFFFFFFFF; // Assuming white color for simplicity, adjust as needed
            }
        }
    }
}

void kmain() {
    terminal_initialize();
    for (int i = 0; i < 240; i++) 
        terminal_putchar('A');

    terminal_putchar('\n');
    for (int i = 0; i < 240; i++) 
        terminal_putchar('b');
    terminal_putchar('\n');
    for (int i = 0; i < 240; i++) 
        terminal_putchar('C');
    terminal_putchar('\n');
    for (int i = 0; i < 240; i++) 
        terminal_putchar('d');
    terminal_putchar('\n');
    for (int i = 0; i < 240; i++) 
        terminal_putchar('E');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('\n');
    terminal_putchar('E');
    terminal_putchar('\n');
    terminal_putchar('r');


	// terminal_initialize();	// clear screen
	// char *a = "kernel";
	// printf("Hello, ");
	// printfc(3, "%s", a);
	// printf(" World!\n\n");

	// uint8_t ret;

	// init_gdt();			// initialize global descriptor table
	// init_idt();			// initialize interrupt descriptor table
	// ACPI_init();		// detect some ACPI tables
	// ret = PS2_init();	// initialize PS/2 controller

	// if (ret) {
	// 	printfc(4, "failed");
	// 	halt_processor();
	// }

	// keyboard_init();	// install keyboard irq handler
	// PIT_init();			// initialize programmable interrupt timer

	// // test syscalls
	// __asm__ __volatile__ ("movl $0, %eax; int $0x80");
	// __asm__ __volatile__ ("movl $1, %eax; int $0x80");

	// initialize_memory();	// initialize physical memory manager
	// printf("\n");

	// ret = initialize_virtual_memory();	// initialize virtual memory

	// if (ret) {
	// 	printf("Error initializing the virtual memory manager!\n");
    //     halt_processor();
	// }

    // ret = fs_init();    // initialize the file system

    // if (ret) {
    //     printf("Error initializing the file system\n");
    //     halt_processor();
    // }

	// // TODO: possible test for paging: see if uint8_t* value at KERNEL_ADDRESS
	// // is the same as 0xC0000000

	// printf("Welcome to MyOS!\n");
	// shell_init();

    // // test kmalloc and kfree
    // //int *p = (int*)kmalloc(1);
    // //printf("got the virtual address: %x\n", p);
    // //int* ap = kmalloc(2);
    // //printf("got the virtual address: %x\n", ap);
    // //kfree(p);
    // //int *t = kmalloc(1);
    // //printf("got the virtual address: %x\n", t);
    // //kfree(ap);
    // //ap = kmalloc(999);
    // //printf("got the virtual address: %x\n", ap);
    // //kfree(ap);
    // //kfree(t);
}
