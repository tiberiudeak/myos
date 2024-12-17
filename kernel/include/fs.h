#ifndef FS_H
#define FS_H 1

#include <stdint.h>

#define FS_BLOCK_SIZE		4096
#define FS_SECTOR_SIZE		512

#define MAX_PATH_LENGTH     1024
#define MAX_OPEN_FILES      256

typedef enum {
	FILETYPE_FILE			= 0x0,
	FILETYPE_DIR			= 0x1
} FS_FILETYPES;

// sizeof boot block: 4096B
struct boot_block {
	uint8_t sectors[8][FS_SECTOR_SIZE];		// 8 sectors make a block (4K)
} __attribute__ ((packed));

// sizeof superblock: 64B
struct superblock {
	uint32_t total_inodes;					// total number of inodes
	uint16_t inode_bitmap_blocks;			// total number of blocks for the inode bitmap
	uint16_t data_bitmap_blocks;			// total number of blocks for the data bitmap
	uint16_t first_inode_bitmap_block;		// starting block for the inode bitmap
	uint16_t first_data_bitmap_block;		// starting block for the data bitmap
	uint32_t first_inode_block;				// first inode block
	uint32_t first_data_block;				// first data block
	uint16_t inode_blocks;					// total number of inode blocks
	uint16_t data_blocks;					// total number of data blocks
	uint8_t extents_per_inode;				// number of direct entents per inode
	uint32_t first_free_inode_bit;			// first free bit in the inode bitmap
	uint32_t first_free_data_bit;			// first free bit in the data bitmap

	uint8_t padding[31];
} __attribute__ ((packed));

// sizeof extent block: 8B
struct extent_block {
	uint32_t first_block;
	uint32_t length;						// length in blocks
} __attribute__ ((packed));

struct fs_datetime {
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hour;
	uint8_t day;
	uint8_t month;
	uint16_t year;
	uint8_t padding;
} __attribute__ ((packed));

// sizeof inode: 64B => one block can have 64 inodes
struct inode_block {
	uint32_t id;
	uint8_t file_type;
	uint32_t size_bytes;
	uint32_t size_sectors;
	struct extent_block extent[4];
	uint32_t single_indirect_block;
	struct fs_datetime datetime;
	uint16_t reference_number;

	uint8_t padding[5];
} __attribute__ ((packed));

// sizeof directory: 64B
struct directory_entry {
	uint32_t id;		// should be the same with the inode's id
	uint8_t name[60];
} __attribute__ ((packed));

// sizeof open files table: 16B
struct open_files_table {
    uint32_t *address;               // virtual address - where file is loaded
    uint32_t offset;                // offset from base address
    struct inode_block *inode;           // file's inode
    uint16_t flags;
    uint16_t reference_number;
} __attribute__ ((packed));


/**
 * @brief Convert bytes to blocks
 *
 * Convert the given nuber of bytes to blocks (4K)
 *
 * @param bytes Number of bytes
 */
static uint32_t bytes_to_blocks(uint32_t bytes) {
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
static uint32_t bytes_to_sectors(uint32_t bytes) {
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

void print_superblock_info(void);
void ls_root_dir(void);
uint8_t fs_init(void);
uint8_t fs_print_dir(void);
char *get_current_path(void);
void* init_open_files_table(void);
void* init_open_inodes_table(void);
struct inode_block get_inode_from_path(char*);
uint8_t load_file(struct inode_block *, uint32_t);
struct inode_block create_file(char*);
uint8_t update_inode_data_disk(struct inode_block*);
uint8_t update_data_block_disk(struct inode_block *, uint32_t);

#endif /* !FS_H */

