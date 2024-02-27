/**
 * Create the disk image with the OS
*/
#include <stdio.h>
#include <stdint.h>
#include "kernel/include/fs.h"

#define INODE_BITMAP_BLOCKS(num_files) (num_files / (FS_BLOCK_SIZE * 8))

int main(int argc, char *argv[]) {
	typedef struct {
		char name[60];
		uint32_t size;
		FILE *fp;
	} file_pointer_type;

	file_pointer_type files[] = {
		{"boot/bootloader.bin", 0, NULL},
		{"kernel/kernel.bin", 0, NULL}
	};

	char image_name[] = "test.bin";
	FILE *image_fp = fopen(image_name, "wb");

	const uint32_t num_files = sizeof files / sizeof files[0];
	printf("total files in the image: %d\n", num_files);

	for (uint32_t i = 0; i < num_files; i++) {
		files[i].fp = fopen(files[i].name, "rb");
		fseek(files[i].fp, 0, SEEK_END);
		files[i].size = ftell(files[i].fp);
		rewind(files[i].fp);
	}

	// write boot block
	boot_block_t boot_block = {0};

	// read the first sector of the bootloader
	int ret = fread((void*)boot_block.sectors[0], FS_SECTOR_SIZE, 1, files[0].fp);

	if (ret == 0) {
		printf("Error reading bootloader\n");
		return 1;
	}

	ret = fwrite(boot_block.sectors[0], FS_SECTOR_SIZE, 1, image_fp);

	if (ret == 0) {
		printf("Error writing to file\n");
		return 1;
	}

	// write second stage bootloader sector
	ret = fread((void*)boot_block.sectors[1], FS_SECTOR_SIZE, 1, files[0].fp);

	if (ret == 0) {
		printf("Error reading bootloader\n");
		return 1;
	}

	ret = fwrite(boot_block.sectors[1], FS_SECTOR_SIZE, 1, image_fp);

	if (ret == 0) {
		printf("Error writing to file\n");
		return 1;
	}

	// fill rest of filesystem block with 0
	for (size_t i = 2; i < 8; i++) {
		fwrite(boot_block.sectors[i], FS_SECTOR_SIZE, 1, image_fp);
	}


	// write super block
	superblock_t superblock = {0};

	superblock.total_inodes = num_files + 2; // reserved and root directory
	superblock.num_inode_bitmap_blocks = num_files / (FS_BLOCK_SIZE * 8) +
		((num_files % (FS_BLOCK_SIZE * 8) > 0) ? 1 : 0);
	superblock.num_data_bitmap_blocks;
	superblock.first_inode_bitmap_block = 2;
	superblock.first_data_bitmap_block;
	superblock.first_inode_block;
	superblock.first_data_block;
	superblock.max_file_size_bytes;
	superblock.block_size_bytes;
	superblock.inode_size_bytes;
	superblock.num_inode_blocks;
	superblock.num_data_blocks;
	superblock.root_inode_pointer;
	superblock.inodes_per_block;
	superblock.direct_entents_per_inode;
	superblock.extents_per_indorect_block;
	superblock.first_free_inode_bit;			// used when creating a file
	superblock.first_free_data_bit;			// used when allocating data
	superblock.first_unreserved_inode;
	superblock.reference_number;
	// write inode bitmap

	// write data bitmap

	// write inodes

	// write data


	for (size_t i = 0; i < num_files; i++) {
		fclose(files[i].fp);
	}

	return 0;
}
