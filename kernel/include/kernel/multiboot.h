#ifndef MULTIBOOT_H
#define MULTIBOOT_H 1

#include <stdint.h>

#define MULTIBOOT_BOOTLOADER_MAGIC		0x2BADB002

#define MULTIBOOT_INFO_MEMORY			0x00000001
#define MULTIBOOT_INFO_BOOTDEV			0x00000002
#define MULTIBOOT_INFO_CMDLINE			0x00000004
#define MULTIBOOT_INFO_MODS				0x00000008
#define MULTIBOOT_INFO_SYMS				0x00000010
#define MULTIBOOT_INFO_ELF_SHDR			0x00000020
#define MULTIBOOT_INFO_MEMMAP			0x00000040
#define MULTIBOOT_INFO_DRIVE			0x00000080
#define MULTIBOOT_INFO_CONFTABLE		0x00000100
#define MULTIBOOT_INFO_BOOTLOADNAME		0x00000200
#define MULTIBOOT_INFO_APM_TABLE		0x00000400
#define MULTIBOOT_INFO_VBE				0x00000800
#define MULTIBOOT_INFO_FRAMEBUFFER		0x00001000

struct multiboot_sym_table {
	uint32_t tabsize;
	uint32_t strsize;
	uint32_t addr;
	uint32_t reserved;
};

struct multiboot_elf_shdr {
	uint32_t num;
	uint32_t size;
	uint32_t addr;
	uint32_t shndx;
};

struct multiboot_mod_list {
	uint32_t mod_start;
	uint32_t mod_end;
	uint32_t string;
	uint32_t reserved;
};

struct multiboot_mmap_entry {
	uint32_t size;
	uint64_t base_addr;
	uint64_t length;
	uint32_t type;
}__attribute__((packed));

struct multiboot_drive_entry {
	uint32_t size;
	uint8_t drive_number;
	uint8_t drive_mode;
	uint16_t drive_cylinders;
	uint8_t drive_heads;
	uint8_t drive_sectors;
	uint8_t drive_ports;
}__attribute__((packed));

struct multiboot_color {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};

struct multiboot_info {
	uint32_t flags;
	uint32_t mem_lower;
	uint32_t mem_upper;
	uint32_t boot_device;
	uint32_t cmdline;
	uint32_t mods_count;
	uint32_t mods_addr;

	union {
		struct multiboot_sym_table sym_table;
		struct multiboot_elf_shdr elf_shdr;
	} u;

	uint32_t mmap_length;
	uint32_t mmap_addr;

	uint32_t drive_length;
	uint32_t drive_addr;

	uint32_t config_table;
	uint32_t bootloader_name;
	uint32_t apm_table;

	uint32_t vbe_control_info;
	uint32_t vbe_mode_info;
	uint16_t vbe_mode;
	uint16_t vbe_interface_seg;
	uint16_t vbe_interface_off;
	uint16_t vbe_interface_len;

	uint64_t framebuffer_addr;
	uint32_t framebuffer_pitch;
	uint32_t framebuffer_width;
	uint32_t framebuffer_height;
	uint8_t framebuffer_bpp;
	uint8_t framebuffer_type;
	union {
		struct {
			uint32_t framebuffer_palette_addr;
			uint16_t framebuffer_palette_num_colors;
		};
		struct {
			uint8_t framebuffer_red_field_position;
			uint8_t framebuffer_red_mask_size;
			uint8_t framebuffer_green_field_position;
			uint8_t framebuffer_green_mask_size;
			uint8_t framebuffer_blue_field_position;
			uint8_t framebuffer_blue_mask_size;
		};
	};
};

#endif
