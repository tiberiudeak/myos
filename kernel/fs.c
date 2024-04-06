#include <fs.h>
#include <global_addresses.h>
#include <disk/disk.h>
#include <mm/kmalloc.h>
#include <kernel/tty.h>

#include <stddef.h>
#include <stdio.h>
#include <string.h>

superblock_t *superblock = (superblock_t*)SUPERBLOCK_ADDRESS;
inode_block_t current_directory;
inode_block_t parent_directory;
char current_path[MAX_PATH_LENGTH];

void print_superblock_info(void) {
    printk("memory layout information (from the superblock):\n");

    printk("\ttotal inodes: %d\n", superblock->total_inodes);
    printk("\tfirst block with inodes: %d\n", superblock->first_inode_block);
    printk("\tnumber of bocks with inodes: %d\n", superblock->inode_blocks);
    printk("\tinodes bitmap blocks: %d\n", superblock->inode_bitmap_blocks);
    printk("\tfirst inode bitmap block: %d\n", superblock->first_inode_bitmap_block);

    printk("\tfirst data block: %d\n", superblock->first_data_block);
    printk("\tnumber of blocks with data: %d\n", superblock->data_blocks);
    printk("\tdata bitmap blocks: %d\n", superblock->data_bitmap_blocks);
    printk("\tfirst data bitmap block: %d\n", superblock->first_data_bitmap_block);
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
            printk("error loading block from disk\n");
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
        printk("out of memory\n");
        return result;
    }

    // go through each block with inodes and load it
    for (int i = 0; i < superblock->inode_blocks; i++) {
        uint32_t starting_sector = (superblock->first_inode_block + i) *
                                    (FS_BLOCK_SIZE / FS_SECTOR_SIZE);
        uint32_t number_of_sector = FS_BLOCK_SIZE / FS_SECTOR_SIZE; // one full block

        int ret = read_sectors(starting_sector, number_of_sector, (uint32_t)addr);

        if (ret) {
            printk("error loading block from disk\n");
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
        printk("out of memory\n");
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
            printk("file with id %d not found\n", id);
            break;
        }

        if (dir_entry->id != 0) {
            printk("%s ", file_inode.file_type == 1 ? "d" : "f");
            printk(" %d/%d/%d ", file_inode.datetime.day, file_inode.datetime.month,
                    file_inode.datetime.year);
            printk(" %d", file_inode.size_bytes);
            printk("\t%s\n", dir_entry->name);
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
            printk("error loading root block from disk: %d\n", ret);
            return;
        }

        // print content of root dir
        // max entries is FS_BLOCK_SIZE / sizeof(directory_entry_t)
        int limit = FS_BLOCK_SIZE / sizeof(directory_entry_t);

        for (int i = 0; i < limit; i++) {
            directory_entry_t *dir_entry = (directory_entry_t*)addr + i;

            if (dir_entry->id != 0) {
                printk("%s\n", dir_entry->name);
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
            printk("error loading block from disk\n");
            return 1;
        }

        // go to inode with id 1 (root dir) (second inode in the block)
        inode_block_t *root_dir_inode = (inode_block_t*) ((void*)addr + sizeof(inode_block_t));

        current_directory = *root_dir_inode;
        parent_directory = current_directory;   // parent of root is root
        strcpy(current_path, "/");  // initial path is the root direcotry

        kfree(addr);

        return 0;
    }

    return 1;
}


// get inode of the requested file (last file in the given path)
inode_block_t get_inode_from_path(char *path) {
    
    char *character = path;
    inode_block_t current_directory_copy = current_directory;
    inode_block_t parent_directory_copy = parent_directory;
    int file_found = 0;

    if (*character == '/') {
        current_directory_copy = get_inode_from_id(1);   // root has always id 1
        parent_directory_copy = current_directory_copy;
    }

    while (*character != '\0') {
        if (*character == '/') {
            character++;
            continue;
        }

        if (*character == '.' && (*(character+1) == '/' || *(character+1) == '\0')) {
            character++;
            continue;
        }

        if (strncmp(character, "..", 2) == 0) {
            // current directory -> parent directory
            // parent directory -> get inode from the id of ".."
            current_directory_copy = parent_directory_copy;

            
            uint32_t needed_bytes = bytes_to_blocks(current_directory_copy.size_bytes) * FS_BLOCK_SIZE;
            int ret;

            void *addr = kmalloc(needed_bytes);

            if (addr == NULL) {
                printk("out of memory\n");
                return (inode_block_t){0};
            }

            // load data blocks containing direcory entries
            ret = load_file(&current_directory_copy, (uint32_t)addr);

            if (ret) {
                kfree(addr);
                return (inode_block_t){0};
            }

            int limit = (FS_BLOCK_SIZE / sizeof(directory_entry_t)) * bytes_to_blocks(needed_bytes);

            for (int i = 0; i < limit; i++) {
                directory_entry_t *dir_entry = (directory_entry_t*)addr + i;

                if (strcmp((char*)dir_entry->name, "..") == 0) {
                    printk("parent inode has id: %d\n", dir_entry->id);
                    parent_directory_copy = get_inode_from_id(dir_entry->id);
                }
            }

            character += 2;
            kfree(addr);
            continue;
        }
        
        // get directory/file name

        char *name = character;
        file_found = 0;

        while (*character != '/' && *character != '\0') {
            character++;
        }

        name[character - name] = '\0';

        // look for the name in the current directory and get the inode
        uint32_t needed_bytes = bytes_to_blocks(current_directory_copy.size_bytes) * FS_BLOCK_SIZE;
        int ret;

        void *addr = kmalloc(needed_bytes);

        if (addr == NULL) {
            printk("out of memory\n");
            return (inode_block_t){0};
        }

        // load data blocks containing direcory entries
        ret = load_file(&current_directory_copy, (uint32_t)addr);

        if (ret) {
            kfree(addr);
            return (inode_block_t){0};
        }

        int limit = (FS_BLOCK_SIZE / sizeof(directory_entry_t)) * bytes_to_blocks(needed_bytes);
        inode_block_t tmp_inode;

        // go through each entry in the directory and search for the file name
        for (int i = 0; i < limit; i++) {
            directory_entry_t *dir_entry = (directory_entry_t*)addr + i;

            if (strcmp((char*)dir_entry->name, name) == 0) {
                file_found = 1;
                if (*character == '/') {
                    // this means that the found file is a directory
                    tmp_inode = get_inode_from_id(dir_entry->id);
                    if (tmp_inode.file_type != FILETYPE_DIR) {
                        kfree(addr);
                        return (inode_block_t){0};
                    }

                    parent_directory_copy = current_directory_copy;
                    current_directory_copy = tmp_inode;
                }
                else {
                    // final file reached!
                    tmp_inode = get_inode_from_id(dir_entry->id);
                    if (tmp_inode.file_type == FILETYPE_DIR) {
                        kfree(addr);
                        return (inode_block_t){0};
                    }

                    kfree(addr);
                    return tmp_inode;
                }
            }
        }

        if (file_found == 0) {
            kfree(addr);
            return (inode_block_t){0};
        }

        kfree(addr);
    }


    
    // !!! I can get the parent inode of the current inode by looking (this inodes
    // are inodes representing directories) at the id of "..". I can then get the
    // inode of the found index using the get_inode_from_id() function

    return (inode_block_t){0};
}

inode_block_t create_file(char *path) {

    // add directory entry in the current directory
    return (inode_block_t){0};
}

uint8_t update_inode_data_disk(inode_block_t *inode) {
    uint32_t *tmp_sector = kmalloc(FS_SECTOR_SIZE);
    int ret;

    if (tmp_sector == NULL)
        return 1;

    // 8 = number of sectors per block
    // 8 = inodes per sector
    ret = read_sectors((superblock->first_inode_block * 8) + (inode->id / 8) + 1, 1, (uint32_t)tmp_sector);

    if (ret)
        goto err;

    inode_block_t *tmp_inode = (inode_block_t*) tmp_sector + (inode->id % 8);
    printk("test test: %d\n", tmp_inode->size_bytes);
    *tmp_inode = *inode;

    tmp_inode = (inode_block_t*)tmp_sector + (inode->id % 8);
    printk("test test: %d\n", tmp_inode->size_bytes);

    ret = write_sectors((superblock->first_inode_block * 8) + (inode->id / 8) + 1, 1, (uint32_t)tmp_sector);

    if (ret)
        goto err;

    //ret = read_sectors((superblock->first_inode_block * 8) + (inode->id / 8) + 1, 1, (uint32_t)tmp_sector);

    //if (ret)
    //    goto err;

    //tmp_inode = (inode_block_t*) tmp_sector;
    //printk("test test after write: %d\n", tmp_inode->id);

    kfree(tmp_sector);
    return 0;

err:
    kfree(tmp_sector);
    return 1;
}


uint8_t update_data_block_disk(inode_block_t *inode, uint32_t addr) {
    uint32_t nr_blocks = bytes_to_blocks(inode->size_bytes);
    int ret;

    for (size_t i = 0; i < superblock->extents_per_inode && nr_blocks > 0; i++) {
        ret = write_sectors(inode->extent[i].first_block * 8,
                inode->extent[i].length * 8,
                addr);

        if (ret)
            return -1;
    }

    return 0;
}

void* init_open_files_table(void) {
    open_files_table_t *tmp = (open_files_table_t*) kmalloc(sizeof(open_files_table_t) * MAX_OPEN_FILES);

    if (tmp == NULL) {
        printk("out of memory\n");
        return NULL;
    }

    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        tmp[i] = (open_files_table_t){0};
    }

    return tmp;
}

void* init_open_inodes_table(void) {
    inode_block_t *tmp = (inode_block_t*) kmalloc(sizeof(inode_block_t) * MAX_OPEN_FILES);

    if (tmp == NULL) {
        printk("out of memory\n");
        return NULL;
    }

    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        tmp[i] = (inode_block_t){0};
    }

    return tmp;
}

