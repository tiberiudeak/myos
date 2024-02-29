/**
 * Create the disk image with the OS
*/
#include <stdio.h>
#include <stdint.h>
#include "kernel/include/fs.h"

typedef struct {
	char name[60];
	uint32_t size;
	FILE *fp;
} file_pointer_type;

int write_boot_block(file_pointer_type files[], FILE *image_fp) {

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

	return 0;
}

int get_disk_size(file_pointer_type files[], int num_files) {
	int size = 0;

	// TODO: maybe don't count the bootloader
	for (size_t i = 0; i < num_files; i++) {
		size += files[i].size;
	}

	return size;
}

int write_superblock(file_pointer_type files[], FILE *image_fp, int num_files, int total_file_blocks) {

	// write super block
	superblock_t superblock = {0};

	// the number of files (without the bootloader), 0 reserved and 1 root
	superblock.total_inodes = num_files + 1;

	// inode bitmap block is 2 (0:boot, 1:superblock)
	superblock.first_inode_bitmap_block = 2;

	// set number of blocks for inode bitmap to 1 (TODO: this will have to change)
	superblock.num_inode_bitmap_blocks = 1;

	// first data bitmap block is after the last block with inode bitmap
	superblock.first_data_bitmap_block = superblock.first_inode_bitmap_block +
									superblock.num_inode_bitmap_blocks;

	// get total disk size, then see to how many blocks it converts to (the number of
	// blocks will be the number of bits in the bitmap)
	uint32_t disk_size = get_disk_size(files, num_files);
	uint32_t data_blocks = bytes_to_blocks(disk_size);
	superblock.num_data_bitmap_blocks = data_blocks / (FS_BLOCK_SIZE * 8) +
			((data_blocks % (FS_BLOCK_SIZE * 8) > 0) ? 1 : 0);

	superblock.num_inode_blocks = bytes_to_blocks(superblock.total_inodes * sizeof(inode_block_t));

	superblock.first_inode_block = superblock.first_data_bitmap_block + superblock.num_data_bitmap_blocks;
	superblock.first_data_block = superblock.first_inode_block + superblock.num_inode_blocks;
	superblock.max_file_size_bytes = 0xFFFFFFFF;
	superblock.block_size_bytes = FS_BLOCK_SIZE;
	superblock.inode_size_bytes = sizeof(inode_block_t);



	// total blocks for the files and one for the root dir
	superblock.num_data_blocks = total_file_blocks + 1;

	// at runtime
	superblock.root_inode_pointer = 0;
	superblock.inodes_per_block = FS_BLOCK_SIZE / sizeof(inode_block_t);
	superblock.direct_entents_per_inode = 4;
	superblock.extents_per_indirect_block = FS_BLOCK_SIZE / sizeof(extent_block_t);
	superblock.first_free_inode_bit = superblock.num_inode_blocks;			// used when creating a file
	superblock.first_free_data_bit = total_file_blocks + 1;			// used when allocating data
	superblock.first_unreserved_inode = 2;

	printf("superblock information:\n");
	printf("\ttotal inodes: %d\n", superblock.total_inodes);
	printf("\tfirst inode bitmap block: %d\n", superblock.first_inode_bitmap_block);
	printf("\tnum inode bitmap blocks: %d\n", superblock.num_inode_bitmap_blocks);
	printf("\tfirst data bitmap block: %d\n", superblock.first_data_bitmap_block);
	printf("\tnum data bitmap blocks: %d\n", superblock.num_data_bitmap_blocks);
	printf("\tfirst inode block: %d\n", superblock.first_inode_block);
	printf("\tfirst data block: %d\n", superblock.first_data_block);
	printf("\tmax file size: %d bytes\n", superblock.max_file_size_bytes);
	printf("\tblock size bytes: %d\n", superblock.block_size_bytes);
	printf("\tinode size bytes: %d\n", superblock.inode_size_bytes);
	printf("\tnum inode blocks: %d\n", superblock.num_inode_blocks);
	printf("\tnum data blocks: %d\n", superblock.num_data_blocks);
	printf("\tinodes per block: %d\n", superblock.inodes_per_block);
	printf("\tdirect entents per inode: %d\n", superblock.direct_entents_per_inode);
	printf("\tindirect extents: %d\n", superblock.extents_per_indirect_block);
	printf("\tfirst free inode bit: %d\n", superblock.first_free_inode_bit);
	printf("\tfirst free data block: %d\n", superblock.first_free_data_bit);
	printf("\tfirst unreserved inode: %d\n", superblock.first_unreserved_inode);

	int	ret = fwrite(&superblock, sizeof(superblock_t), 1, image_fp);

	if (ret == 0) {
		printf("Error writing the superblock\n");
		return 1;
	}

	// padding
	superblock_t padding = {0};
	// ret = fwrite(&padding, sizeof(superblock_t), 63, image_fp);

	if (ret == 0) {
		printf("Error writing the superblock\n");
		return 1;
	}

	return 0;
}

int main(int argc, char *argv[]) {

	file_pointer_type files[] = {
		{"boot/bootloader.bin", 0, NULL},
		{"kernel/kernel.bin", 0, NULL}
	};

	char image_name[] = "test.bin";
	FILE *image_fp = fopen(image_name, "wb");
	uint32_t total_file_blocks = 0;

	printf("Creating the OS disk image %s...\n", image_name);

	const uint32_t num_files = sizeof files / sizeof files[0];
	printf("total files in the image: %d\n", num_files);

	for (uint32_t i = 0; i < num_files; i++) {
		files[i].fp = fopen(files[i].name, "rb");
		fseek(files[i].fp, 0, SEEK_END);
		files[i].size = ftell(files[i].fp);
		rewind(files[i].fp);

		total_file_blocks += bytes_to_blocks(files[i].size);
	}

	for (uint32_t i = 0; i < num_files; i++) {
		printf("\t%s - size: %d bytes\n", files[i].name, files[i].size);
	}

	printf("total disk size: %d bytes\n", get_disk_size(files, num_files));

	// create boot block
	int ret = write_boot_block(files, image_fp);

	if (ret) {
		printf("Error creating the boot block\n");
		return 1;
	}

	// create superblock
	ret = write_superblock(files, image_fp, num_files, total_file_blocks);

	if (ret) {
		printf("Error creating the superblock\n");
		return 1;
	}

	// write inode bitmap

	// write data bitmap

	// write inodes

	// write data


	for (size_t i = 0; i < num_files; i++) {
		fclose(files[i].fp);
	}

	return 0;
}
