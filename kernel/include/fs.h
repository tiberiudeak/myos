#ifndef FS_H
#define FS_H

#include <stdint.h>

#define FS_BLOCK_SIZE		4096
#define FS_SECTOR_SIZE		512

typedef enum {
	FILETYPE_FILE			= 0x0,
	FILETYPE_DIR			= 0x1
} FS_FILETYPES;

typedef struct {
	uint8_t sectors[8][FS_SECTOR_SIZE];	// 8 sectors make a block (4K)
} __attribute__ ((packed)) boot_block_t;

typedef struct {
	uint32_t total_inodes;
	uint16_t num_inode_bitmap_blocks;
	uint16_t num_data_bitmap_blocks;
	uint16_t first_inode_bitmap_block;
	uint16_t first_data_bitmap_block;
	uint32_t first_inode_block;
	uint32_t first_data_block;
	uint32_t max_file_size_bytes;
	uint16_t block_size_bytes;
	uint8_t inode_size_bytes;
	uint16_t num_inode_blocks;
	uint16_t num_data_blocks;
	uint32_t root_inode_pointer;
	uint8_t inodes_per_block;
	uint8_t direct_entents_per_inode;
	uint16_t extents_per_indorect_block;
	uint32_t first_free_inode_bit;			// used when creating a file
	uint32_t first_free_data_bit;			// used when allocating data
	uint8_t first_unreserved_inode;
	uint16_t reference_number;

	uint8_t padding[14];
} __attribute__ ((packed)) superblock_t;

typedef struct {
	uint32_t first_block;
	uint32_t length;
} __attribute__ ((packed)) extent_block_t;

typedef struct {
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hour;
	uint8_t day;
	uint8_t month;
	uint16_t year;
	uint8_t padding;
} __attribute__ ((packed)) fs_datetime_t;

typedef struct {
	uint32_t id;		// unique
	uint8_t type;		// file type
	uint32_t size_bytes;
	uint32_t size_sectots;
	extent_block_t extend[4];
	uint32_t single_indirect_block;
	fs_datetime_t datetime;

	uint8_t padding[7];
} __attribute__ ((packed)) inode_block_t;


typedef struct {
	uint32_t id;
	uint8_t name[60];
} __attribute__ ((packed)) directory_entry_t;

/**
 * @brief Convert bytes to blocks
 *
 * Convert the given nuber of bytes to blocks (4K)
 *
 * @param bytes Number of bytes
 */
uint32_t bytes_to_blocks(uint32_t bytes) {
	if (bytes == 0) {
		return 0;
	}

	if (bytes < FS_BLOCK_SIZE) {
		return 1;
	}

	if (bytes % FS_BLOCK_SIZE != 0) {
		return (bytes / FS_BLOCK_SIZE) + 1;
	}

	return bytes / FS_BLOCK_SIZE;
}

/**
 * @brief Convert bytes to sectors
 *
 * Convert the given nuber of bytes to sectors (4K)
 *
 * @param bytes Number of bytes
 */
uint32_t bytes_to_sectors(uint32_t bytes) {
	if (bytes == 0) {
		return 0;
	}

	if (bytes < FS_SECTOR_SIZE) {
		return 1;
	}

	if (bytes % FS_SECTOR_SIZE != 0) {
		return (bytes / FS_SECTOR_SIZE) + 1;
	}

	return bytes / FS_SECTOR_SIZE;
}
#endif /* !FS_H */
