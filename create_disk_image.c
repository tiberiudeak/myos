/**
 * Create the disk image with the OS. Image layout:
 *
 * |--------|------------|---------------|--------------|--------|--------|
 * | Boot   | Superblock | Inode Bitmap  | Data Bitmap  | Inode  | Data   |
 * | block  |            |    blocks     |    blocks    | blocks | blocks |
 * |--------|------------|---------------|--------------|--------|--------|
 *
 * There will be only one boot block (so the bootloader has to fit in 4K)
 * and one superblock.
 *
 * Block size: 	4096B
 * Sector size:	512B
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "kernel/include/fs.h"

typedef struct {
	char name[60];
	uint32_t size;
	FILE *fp;
} file_pointer_type;

/**
 * @brief Get number of bytes needed for padding until the given limit is reached
 *
 * This function returns the number of bytes that are needed for padding until
 * the given limit is reached.
 *
 * @param bytes Current number of bytes
 * @param limit Limit to be reached
 *
 * @return Number of bytes until the limit is reached
 */
uint32_t padding_bytes(uint32_t bytes, uint32_t limit) {
	return bytes % limit == 0 ? 0 : (bytes - (bytes % limit) + limit) - bytes;
}

/**
 * @brief Write boot block in the disk image
 *
 * This function writes the first block in the disk image. Currently, it writes the
 * first and second sectors (that make the entire bootloader) and adds padding for the
 * remaining bytes.
 *
 * @param files 	Array of files included in the image
 * @param image_fp 	File pointer to the disk image
 *
 * @return 1 if error occured, 0 otherwise
 */
int write_boot_block(file_pointer_type files[], FILE *image_fp) {
	uint32_t written_bytes = 0;
	boot_block_t boot_block = {0};

	// read the first sector of the bootloader
	int ret = fread((void*)boot_block.sectors[0], FS_SECTOR_SIZE, 1, files[0].fp);

	if (ret == 0) {
		printf("Error reading bootloader\n");
		return 1;
	}

	// write first sector to image
	ret = fwrite(boot_block.sectors[0], FS_SECTOR_SIZE, 1, image_fp);

	if (ret == 0) {
		printf("Error writing to file\n");
		return 1;
	}

	written_bytes += FS_SECTOR_SIZE;

	// read second stage bootloader sectors (now including the VESA BIOS Ext.)
	ret = fread((void*)boot_block.sectors[1], FS_SECTOR_SIZE, 3, files[0].fp);

	if (ret == 0) {
		printf("Error reading bootloader\n");
		return 1;
	}

	// write second sector to image
	ret = fwrite(boot_block.sectors[1], FS_SECTOR_SIZE, 3, image_fp);

	if (ret == 0) {
		printf("Error writing to file\n");
		return 1;
	}

	written_bytes += 3 * FS_SECTOR_SIZE;

	// read third stage bootloader sector
	ret = fread((void*)boot_block.sectors[2], FS_SECTOR_SIZE, 1, files[0].fp);

	if (ret == 0) {
		printf("Error reading bootloader\n");
		return 1;
	}

	// write third sector to image
	ret = fwrite(boot_block.sectors[2], FS_SECTOR_SIZE, 1, image_fp);

	if (ret == 0) {
		printf("Error writing to file\n");
		return 1;
	}

	written_bytes += FS_SECTOR_SIZE;

	// fill rest of boot block with 0
	uint32_t padding = padding_bytes(written_bytes, FS_BLOCK_SIZE);
	uint8_t zero[FS_BLOCK_SIZE] = {0};

	ret = fwrite(&zero, sizeof(uint8_t), padding, image_fp);

	if (ret == 0) {
		printf("Error writing to file\n");
		return 1;
	}

	return 0;
}

/**
 * @brief Return size in bytes for the files except the bootloader
 *
 * This function goes through the files and computes the total site (in bytes).
 *
 * @param files 	Array of files
 * @param num_files The number of files in the array
 *
 * @return Total size in bytes
 */
int get_disk_size(file_pointer_type files[], int num_files) {
	int size = 0;

	// this doesn't take into consideration the bootloader
	for (size_t i = 1; i < num_files; i++) {
		size += files[i].size;
	}

	return size;
}

/**
 * @brief Write superblock to disk image
 *
 * This function writes the superblock to the final disk image. For more details about the
 * information in the superblock, see comments below.
 *
 * @param superblock 		Pointer to a superblock struct
 * @param files 			Array of files to be included in the final image
 * @param image_fp			File pointer to the disk image
 * @param num_files			Number of files in the files array
 * @param total_file_blocks	Total number of data blocks needed by the files
 *
 * @return 1 if error occured, 0 otherwise
 */
int write_superblock(superblock_t *superblock, file_pointer_type files[], FILE *image_fp, int num_files, int total_file_blocks) {
	// the number of files (without the bootloader), 0 reserved and 1 root (num_files - 1 + 2)
	superblock->total_inodes = num_files + 1;

	// inode bitmap block is 2 (0:boot, 1:superblock)
	superblock->first_inode_bitmap_block = 2;

	// set number of blocks for inode bitmap to 1 (TODO: this will have to change)
	superblock->inode_bitmap_blocks = 1;

	// first data bitmap block is after the last block with inode bitmap
	superblock->first_data_bitmap_block = superblock->first_inode_bitmap_block +
									superblock->inode_bitmap_blocks;

	// get total disk size, then see to how many blocks it converts to (the number of
	// blocks will be the number of bits in the bitmap)
	uint32_t disk_size = get_disk_size(files, num_files);
	uint32_t data_blocks = bytes_to_blocks(disk_size) + 1; // +1 for the root directory block
	superblock->data_bitmap_blocks = data_blocks / (FS_BLOCK_SIZE * 8) +
			((data_blocks % (FS_BLOCK_SIZE * 8) > 0) ? 1 : 0);

	superblock->inode_blocks = bytes_to_blocks(superblock->total_inodes * sizeof(inode_block_t));

	superblock->first_inode_block = superblock->first_data_bitmap_block + superblock->data_bitmap_blocks;
	superblock->first_data_block = superblock->first_inode_block + superblock->inode_blocks;

	// total blocks for the files and one for the root dir (it shoul be +1
	// but I don't want to create a block for the bootloader, so it would be -1)
	superblock->data_blocks = total_file_blocks;

	superblock->extents_per_inode = 4;
	superblock->first_free_inode_bit = superblock->inode_blocks;			// used when creating a file
	superblock->first_free_data_bit = total_file_blocks + 1;			// used when allocating data

	printf("superblock information:\n");
	printf("\ttotal inodes: %d\n", superblock->total_inodes);
	printf("\tfirst inode bitmap block: %d\n", superblock->first_inode_bitmap_block);
	printf("\tnum inode bitmap blocks: %d\n", superblock->inode_bitmap_blocks);
	printf("\tfirst data bitmap block: %d\n", superblock->first_data_bitmap_block);
	printf("\tnum data bitmap blocks: %d\n", superblock->data_bitmap_blocks);
	printf("\tfirst inode block: %d\n", superblock->first_inode_block);
	printf("\tfirst data block: %d\n", superblock->first_data_block);
	printf("\tnum inode blocks: %d\n", superblock->inode_blocks);
	printf("\tnum data blocks: %d\n", superblock->data_blocks);
	printf("\tdirect extents per inode: %d\n", superblock->extents_per_inode);
	printf("\tfirst free inode bit: %d\n", superblock->first_free_inode_bit);
	printf("\tfirst free data bit: %d\n", superblock->first_free_data_bit);

	// write superblock to image
	int	ret = fwrite(superblock, sizeof(superblock_t), 1, image_fp);

	if (ret == 0) {
		printf("Error writing the superblock\n");
		return 1;
	}

	// writen bytes are 64B
	uint32_t written_bytes = sizeof(superblock_t);
	uint32_t padding = padding_bytes(written_bytes, FS_BLOCK_SIZE);
	uint8_t zero[FS_BLOCK_SIZE] = {0};

	ret = fwrite(&zero, sizeof(uint8_t), padding, image_fp);

	if (ret == 0) {
		printf("Error writing the superblock\n");
		return 1;
	}

	return 0;
}

/**
 * @brief Write the inode bitmap to disk image
 *
 * This function writes the inode bitmap to the final disk image.
 * TODO: Currently it only writes one block (4K)
 *
 * @param image_fp		File pointer to the disk image
 * @param superblock	Pointer to the superblock
 *
 * @return 1 if error occured, 0 otherwise
 */
int write_inode_bitmap(FILE *image_fp, superblock_t *superblock) {
	uint8_t sector[FS_BLOCK_SIZE] = {0};
	uint32_t num_inodes = superblock->total_inodes;
	uint32_t *p = (uint32_t*)sector;

	uint32_t full = num_inodes / 32;
	uint32_t left = num_inodes % 32;

	for (size_t i = 0; i < full; i++) {
		*p = 0xFFFFFFFF;
		p++;
	}

	// 2 to the power of number of bits to set minus 1 minus 1 will
	// be the mask
	// example: we want to set 3 bits: (2 ^ (3 - 1)) - 1 = (0b0010 << 2) - 1 =
	// 0b1000 - 1 = 0b0111 (three bits are set)
	*p |= (2 << (left - 1)) - 1;

	int ret = fwrite(sector, sizeof(uint8_t), FS_BLOCK_SIZE, image_fp);

	if (ret == 0) {
		printf("Error writing the inode bitmap\n");
		return 1;
	}

	// TODO: take into consideration superblock.inode_bitmap_blocks

	return 0;
}

/**
 * @brief Write the data bitmap to disk image
 *
 * This function writes the data bitmap to the final disk image.
 * TODO: Currently it only writes one block (4K)
 *
 * @param image_fp		File pointer to the disk image
 * @param superblock	Pointer to the superblock
 *
 * @return 1 if error occured, 0 otherwise
 */
int write_data_bitmap(FILE *image_fp, superblock_t *superblock) {

	uint8_t sector[FS_BLOCK_SIZE] = {0};
	uint32_t num_data_block = superblock->data_blocks;
	uint32_t *p = (uint32_t*)sector;

	uint32_t full = num_data_block / 32;
	uint32_t left = num_data_block % 32;

	for (size_t i = 0; i < full; i++) {
		*p = 0xFFFFFFFF;
		p++;
	}

	// 2 to the power of number of bits to set minus 1 minus 1 will
	// be the mask
	// example: we want to set 3 bits: (2 ^ (3 - 1)) - 1 = (0b0010 << 2) - 1 =
	// 0b1000 - 1 = 0b0111 (three bits are set)
	*p |= (2 << (left - 1)) - 1;

	int ret = fwrite(sector, sizeof(uint8_t), FS_BLOCK_SIZE, image_fp);

	if (ret == 0) {
		printf("Error writing the inode bitmap\n");
		return 1;
	}

	// TODO: take into consideration superblock.data_bitmap_blocks

	return 0;
}

/**
 * @brief Write inodes to the disk image
 *
 * This function writes the inodes in the final image. The first inode is the reserved inode
 * and will be all zeroes, the second one describes the root directory, the third one will be the
 * kernel and the rest the remaining files.
 *
 * @param image_fp		File pointer to the disk image
 * @param num_files		Number of file sin the files array
 * @param superblock	Pointer to the superblock
 * @param files			Array of files
 *
 * @return 1 if error occured, 0 otherwise
 */
int write_inodes(FILE *image_fp, int num_files, superblock_t *superblock, file_pointer_type files[]) {
	uint32_t written_bytes = 0;
	inode_block_t inode = {0};
    time_t t;
    struct tm ts;
    time(&t);
    ts = *localtime(&t);

	// inode 0
	int ret = fwrite(&inode, sizeof(inode_block_t), 1, image_fp);

	if (ret == 0) {
		printf("Error writing first inode\n");
		return 1;
	}

	written_bytes += sizeof(inode_block_t);

	// inode 1: root directory
	inode.id = 1;
	inode.file_type = FILETYPE_DIR;
	inode.size_bytes = sizeof(directory_entry_t) * (num_files - 1); // -1 for the bootloader
	inode.size_sectors = bytes_to_sectors(inode.size_bytes);

	inode.extent[0] = (extent_block_t) {
		.first_block = superblock->first_data_block,
		.length = bytes_to_blocks(inode.size_bytes)
	};

    inode.datetime.day = ts.tm_mday;
    inode.datetime.month = ts.tm_mon + 1;
    inode.datetime.year = ts.tm_year + 1900;

	ret = fwrite(&inode, sizeof(inode_block_t), 1, image_fp);

	if (ret == 0) {
		printf("Error writing root inode\n");
		return 1;
	}

	written_bytes += sizeof(inode_block_t);

	//inode 2 and beyond: kernel and the rest of inodes
	uint32_t id = 1;
	uint32_t current_file_first_block = superblock->first_data_block +
			inode.extent[0].length;

	for (size_t i = 1; i < num_files; i++) {
		inode = (inode_block_t) {0};
		inode.id = ++id;
		inode.file_type = FILETYPE_FILE;
		inode.size_bytes = files[i].size;
		inode.size_sectors = bytes_to_sectors(files[i].size);

        inode.datetime.day = ts.tm_mday;
        inode.datetime.month = ts.tm_mon;
        inode.datetime.year = ts.tm_year + 1900;

		inode.extent[0] = (extent_block_t) {
			.first_block = current_file_first_block,
			.length = bytes_to_blocks(files[i].size)
		};

		ret = fwrite(&inode, sizeof(inode_block_t), 1, image_fp);

		if (ret == 0) {
			printf("Error writing a file inode number %ld\n", i);
			return 1;
		}

		written_bytes += sizeof(inode_block_t);
		current_file_first_block += inode.extent[0].length;
	}

	// padding TODO: take into consideration the number of blocks for inodes
	uint32_t written_inodes = num_files + 1; // -1 + 2 (minus bootloader, add root and reserved)
	uint8_t zero[FS_BLOCK_SIZE] = {0};
	uint32_t padding = padding_bytes(written_bytes, FS_BLOCK_SIZE);

	ret = fwrite(&zero, sizeof(uint8_t), padding, image_fp);

	if (ret == 0) {
		printf("Error writing inode block padding\n");
		return 1;
	}

	return 0;
}

/**
 * @brief Write data blocks to disk image
 *
 * This function writes the data blocks to the disk image. The first data block contains the
 * root directory's data (one directory entry for each file, as well as for . and ..). Starting
 * with the second data block is the kernel data. After that, data for the remaining files.
 *
 * @param image_pt		File pointer to the disk image
 * @param num_files		Number of files in the files array
 * @param superblock	Pointer to the superblock
 * @param files			Array of files
 *
 * @return 1 if error occured, 0 otherwise
 */
int write_data(FILE *image_pt, int num_files, superblock_t *superblock, file_pointer_type files[]) {
	uint32_t written_bytes = 0;

	// write first data block which is the root directory that will contain a directory entry for each file/dir
	directory_entry_t root_dir = {0};

	// '.' current dir entry
	root_dir.id = 1;
	strcpy(root_dir.name, ".");

	int ret = fwrite(&root_dir, sizeof(root_dir), 1, image_pt);

	if (ret == 0) {
		printf("Error writing directory entry in root block\n");
		return 1;
	}
	written_bytes += sizeof(directory_entry_t);

	// '..' parent dir entry
	strcpy(root_dir.name, "..");
	ret = fwrite(&root_dir, sizeof(root_dir), 1, image_pt);

	if (ret == 0) {
		printf("Error writing directory entry in root block\n");
		return 1;
	}
	written_bytes += sizeof(directory_entry_t);

	uint32_t id = 2;
	for (size_t i = 1; i < num_files; i++) {
		root_dir.id = id++;

		strcpy(root_dir.name, files[i].name + 4);

		ret = fwrite(&root_dir, sizeof(root_dir), 1, image_pt);

		if (ret == 0) {
			printf("Error writing directory entry in root block\n");
			return 1;
		}
		written_bytes += sizeof(directory_entry_t);
	}

	// END OF ROOT DIRECTORY BLOCK
	// padding
	uint32_t padding = padding_bytes(written_bytes, FS_BLOCK_SIZE);
	uint8_t zero[FS_BLOCK_SIZE] = {0};

	ret = fwrite(&zero, sizeof(uint8_t), padding, image_pt);

	if (ret == 0) {
		printf("Error padding the root dir block\n");
		return 1;
	}

	written_bytes = 0;



	// writing data from files to image
	uint8_t sector[FS_SECTOR_SIZE] = {0};

	// go through each file
	for (size_t i = 1; i < num_files; i++) {
		uint32_t sectors = bytes_to_sectors(files[i].size);
		written_bytes = 0;

		// write one sector at a time
		for (size_t j = 0; j < sectors; j++) {
			uint32_t read_bytes = fread(sector, 1, sizeof(sector), files[i].fp);
			ret = fwrite(&sector, 1, read_bytes, image_pt);

			if (ret == 0) {
				printf("Error wrinting file data to disk image\n");
				return 1;
			}

			written_bytes += read_bytes;
		}

		memset(sector, 0, sizeof(sector));

		uint32_t padding = padding_bytes(written_bytes, FS_BLOCK_SIZE);

		ret = fwrite(&zero, sizeof(uint8_t), padding, image_pt);

		if (ret == 0) {
			printf("Error wrinting file data to disk image\n");
			return 1;
		}
	}

	return 0;
}

void usage(void) {
	printf("Usage:\n");
	printf("\t./create_disk_image <image_name>\n");
}

int main(int argc, char *argv[]) {

	if (argc != 2) {
		usage();
		return 1;
	}

	file_pointer_type files[] = {
		{"boot/bootloader.bin", 0, NULL},
		{"bin/kernel.bin", 0, NULL},
		{"bin/test.txt", 0, NULL},
		{"bin/pr1.o", 0, NULL}
	};

	char image_name[20];
	strcpy(image_name, argv[1]);
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

	printf("total disk size of actual data (without bootloader): %d bytes\n", get_disk_size(files, num_files));

	// create boot block
	int ret = write_boot_block(files, image_fp);

	if (ret) {
		printf("Error creating the boot block\n");
		return 1;
	}

	// create superblock
	superblock_t superblock = {0};
	ret = write_superblock(&superblock, files, image_fp, num_files, total_file_blocks);

	if (ret) {
		printf("Error creating the superblock\n");
		return 1;
	}

	// write inode bitmap
	ret = write_inode_bitmap(image_fp, &superblock);

	if (ret) {
		printf("Error creating the inode bitmap\n");
		return 1;
	}

	// write data bitmap
	ret = write_data_bitmap(image_fp, &superblock);

	if (ret) {
		printf("Error creating the data bitmap\n");
		return 1;
	}

	// write inodes
	ret = write_inodes(image_fp, num_files, &superblock, files);

	if (ret) {
		printf("Error creating the inodes\n");
		return 1;
	}

	// write data
	ret = write_data(image_fp, num_files, &superblock, files);

	if (ret) {
		printf("Error writing the data blocks\n");
		return 1;
	}

	for (size_t i = 0; i < num_files; i++) {
		fclose(files[i].fp);
	}

	fclose(image_fp);
	return 0;
}
