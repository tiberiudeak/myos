#include <fs.h>
#include <global_addresses.h>
#include <disk/disk.h>
#include <mm/kmalloc.h>

#include <stdio.h>

superblock_t *superblock = (superblock_t*)SUPERBLOCK_ADDRESS;
inode_block_t current_directory;
inode_block_t parent_directory;

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

// print files in current directory (to ls other directory, you'll have to go
// to that one and then ls)
uint8_t fs_print_dir(void) {
    uint32_t needed_bytes = bytes_to_blocks(current_directory.size_bytes) * FS_BLOCK_SIZE;
    int ret;

    void *addr = kmalloc(needed_bytes);

    if (addr == NULL) {
        printf("out of memory\n");
        return 1;
    }

    // load data
    ret = load_file(&current_directory, (uint32_t)addr);

    if (ret) {
        return 1;
    }

    int limit = FS_BLOCK_SIZE / sizeof(directory_entry_t);

    for (int i = 0; i < limit; i++) {
        directory_entry_t *dir_entry = (directory_entry_t*)addr + i;

        if (dir_entry->id != 0) {
            printf("%s\n", dir_entry->name);
        }
    }

    kfree(addr);

    return 0;
}

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

// this function sets the current directory to the root dir
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

        kfree(addr);

        return 0;
    }

    return 1;
}

