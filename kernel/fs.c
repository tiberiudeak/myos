#include <fs.h>
#include <global_addresses.h>
#include <disk/disk.h>
#include <mm/kmalloc.h>

#include <stddef.h>
#include <stdio.h>
#include <string.h>

superblock_t *superblock = (superblock_t*)SUPERBLOCK_ADDRESS;
inode_block_t current_directory;
inode_block_t parent_directory;
char current_path[MAX_PATH_LENGTH];

void print_superblock_info(void) {
    printf("memory layout information (from the superblock):\n");

    printf("\ttotal inodes: %d\n", superblock->total_inodes);
    printf("\tfirst block with inodes: %d\n", superblock->first_inode_block);
    printf("\tnumber of bocks with inodes: %d\n", superblock->inode_blocks);
    printf("\tinodes bitmap blocks: %d\n", superblock->inode_bitmap_blocks);
    printf("\tfirst inode bitmap block: %d\n", superblock->first_inode_bitmap_block);

    printf("\tfirst data block: %d\n", superblock->first_data_block);
    printf("\tnumber of blocks with data: %d\n", superblock->data_blocks);
    printf("\tdata bitmap blocks: %d\n", superblock->data_bitmap_blocks);
    printf("\tfirst data bitmap block: %d\n", superblock->first_data_bitmap_block);
}

/**
 * @brief Load file from disk into main memory
 *
 * This function loads a file given through its inode into main memory.
 * Memory at address has to be reserved prior to this call.
 *
 * @param inode     The file's inode
 * @param address   Location where the file will be loaded
 *
 * @return 1 if error occured, 0 otherwise
 */
uint8_t load_file(inode_block_t *inode, uint32_t address) {
    uint32_t number_of_blocks = bytes_to_blocks(inode->size_bytes);
    uint32_t read_blocks = 0;
    uint32_t offset = 0;
    int ret;

    for (int i = 0; number_of_blocks > read_blocks && i < superblock->extents_per_inode; i++) {
        uint32_t starting_sector = inode->extent[0].first_block *
                                    (FS_BLOCK_SIZE / FS_SECTOR_SIZE) + 1;
        uint32_t number_of_sectors = inode->extent[i].length * (FS_BLOCK_SIZE / FS_SECTOR_SIZE);

        ret = read_sectors(starting_sector, number_of_sectors, address + offset);

        if (ret) {
            printf("error loading block from disk\n");
            return 1;
        }

        read_blocks += inode->extent[i].length;
        offset += (inode->extent[i].length) * FS_BLOCK_SIZE;
    }

    return 0;
}

/**
 * @brief Return inode corresponding to the given id
 *
 * This function loads one block with inodes at a time and searches in the loaded
 * block for the given id. If it is found, the respective inode is returned, otherwise
 * an empty inode is returned.
 *
 * @param id The inode's id
 *
 * @return The inode corresponding to the given id if found, empty inode otherwise
 */
inode_block_t get_inode_from_id(uint32_t id) {
    inode_block_t result = (inode_block_t){0};
    void *addr = kmalloc(FS_BLOCK_SIZE);

    if (addr == NULL) {
        printf("out of memory\n");
        return result;
    }

    // go through each block with inodes and load it
    for (int i = 0; i < superblock->inode_blocks; i++) {
        uint32_t starting_sector = (superblock->first_inode_block + i) *
                                    (FS_BLOCK_SIZE / FS_SECTOR_SIZE);
        uint32_t number_of_sector = FS_BLOCK_SIZE / FS_SECTOR_SIZE; // one full block

        int ret = read_sectors(starting_sector, number_of_sector, (uint32_t)addr);

        if (ret) {
            printf("error loading block from disk\n");
            break;
        }

        // block with inodes is loaded at addr, go through each inode and check id
        for (size_t j = 0; j < FS_BLOCK_SIZE / sizeof(inode_block_t); j++) {
            inode_block_t *current_inode = (inode_block_t*) addr + j;

            if (current_inode->id == id) {
                result = *current_inode;
                break;
            }
        }
    }

    kfree(addr);
    return result;
}

/**
 * @brief Print files in current directory
 *
 * This function loads the data blocks for the current directory, goes through each
 * entry and prints information (also from the file's inode).
 *
 * (currently, if you want to print files in another directlry, you'll have to go
 * to that directlry using cd)
 *
 * @return 1 if error occured, 0 otherwise
 */
uint8_t fs_print_dir(void) {
    uint32_t needed_bytes = bytes_to_blocks(current_directory.size_bytes) * FS_BLOCK_SIZE;
    int ret;

    void *addr = kmalloc(needed_bytes);

    if (addr == NULL) {
        printf("out of memory\n");
        return 1;
    }

    // load data blocks containing direcory entries
    ret = load_file(&current_directory, (uint32_t)addr);

    if (ret) {
        kfree(addr);
        return 1;
    }

    int limit = (FS_BLOCK_SIZE / sizeof(directory_entry_t)) * bytes_to_blocks(needed_bytes);

    for (int i = 0; i < limit; i++) {
        directory_entry_t *dir_entry = (directory_entry_t*)addr + i;

        // file's id
        uint32_t id = dir_entry->id;

        if (id == 0) {
            break;
        }

        // search inode for file id
        inode_block_t file_inode = get_inode_from_id(id);

        if (file_inode.id == 0) {
            printf("file with id %d not found\n", id);
            break;
        }

        if (dir_entry->id != 0) {
            printf("%s ", file_inode.file_type == 1 ? "d" : "f");
            printf(" %d/%d/%d ", file_inode.datetime.day, file_inode.datetime.month,
                    file_inode.datetime.year);
            printf(" %d", file_inode.size_bytes);
            printf("\t%s\n", dir_entry->name);
        }
    }

    kfree(addr);

    return 0;
}

// function replaced by fs_print_dir()
void ls_root_dir(void) {
    // load root dir block from disk (we know that it is the first data block)
    // uint8_t read_sectors(uint8_t starting_sector, uint8_t size, uint32_t addr) {

    int ret;
    uint32_t root_dir_block = superblock->first_data_block;
    uint32_t starting_sector = root_dir_block * (FS_BLOCK_SIZE / FS_SECTOR_SIZE) + 1;
    uint32_t number_of_sectors = FS_BLOCK_SIZE / FS_SECTOR_SIZE;

    void *addr = kmalloc(FS_BLOCK_SIZE);

    if (addr != NULL) {
        ret = read_sectors(starting_sector, number_of_sectors, (uint32_t)addr);

        if (ret) {
            printf("error loading root block from disk: %d\n", ret);
            return;
        }

        // print content of root dir
        // max entries is FS_BLOCK_SIZE / sizeof(directory_entry_t)
        int limit = FS_BLOCK_SIZE / sizeof(directory_entry_t);

        for (int i = 0; i < limit; i++) {
            directory_entry_t *dir_entry = (directory_entry_t*)addr + i;

            if (dir_entry->id != 0) {
                printf("%s\n", dir_entry->name);
            }
        }

        kfree(addr);
    }
}

/**
 * @brief Return current path in the filesystem
 *
 * This function returns the current path starting from the root directory.
 *
 * @return Current path starting with "/"
 */
char *get_current_path(void) {
    return current_path;
}

/**
 * @brief Initialize the file system
 *
 * This function loads the first block with inodes in order to get the inode of the root
 * directory (which is inode 1). It sets the current_directory to that inode and initializes
 * the current_path to "/".
 *
 * @return 1 if error occured, 0 otherwise
 */
uint8_t fs_init(void) {
    // load first inode block from memory and save rood directory node
    uint32_t starting_sector = superblock->first_inode_block * (FS_BLOCK_SIZE / FS_SECTOR_SIZE) + 1;
    uint32_t number_of_sectors = FS_BLOCK_SIZE / FS_SECTOR_SIZE;
    int ret;

    void *addr = kmalloc(FS_BLOCK_SIZE);

    if (addr != NULL) {
        ret = read_sectors(starting_sector, number_of_sectors, (uint32_t)addr);

        if (ret) {
            printf("error loading block from disk\n");
            return 1;
        }

        // go to inode with id 1 (root dir) (second inode in the block)
        inode_block_t *root_dir_inode = (inode_block_t*) ((void*)addr + sizeof(inode_block_t));

        current_directory = *root_dir_inode;
        strcpy(current_path, "/");  // initial path is the root direcotry

        kfree(addr);

        return 0;
    }

    return 1;
}

