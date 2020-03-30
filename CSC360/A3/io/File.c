#define vdisk_path "../disk/vdisk"
#define BLOCK_SIZE 512
#define NUM_BLOCKS 4096
#define POINTERS_PER_BLK 1024
#define NUM_INODES_PER_BLK 128  // This number is up to you
#define POINTERS_PER_INODE 5
#define MAGIC_NUMBER 0xEFBE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk_controller.h"
#include "bitmap.h"

struct superblock* sb;
uint8_t *free_block_vector;

// 32 bytes
struct inode{
    uint32_t file_size;
    uint32_t flags; // 1: available, 0: unavailable
    uint16_t *first_ten_blocks;
    uint16_t single_indirect_block;
    uint16_t double_indirect_block;
};

// 12 bytes
struct superblock{
    uint32_t magic_number;
    uint32_t num_blocks;
    uint32_t num_inodes;
};

struct block{
    struct superblock* sb;
    struct inode i_node[NUM_INODES_PER_BLK];
    
    char data[BLOCK_SIZE];
};

void InitLLFS(){

    // Initialize the disk with all zeros with size 512*4096
    create_vdisk();

    free_block_vector = (uint8_t*) calloc(BLOCK_SIZE, sizeof(uint8_t));

    // -- Initialize Block 0 Superblock --

    sb = (struct superblock*) calloc(1, sizeof(struct superblock));
    sb -> magic_number = MAGIC_NUMBER;
    sb -> num_blocks = NUM_BLOCKS;
    sb -> num_inodes = NUM_INODES_PER_BLK;
    
    // Create a buffer
    char* buffer = (char *) calloc(BLOCK_SIZE, sizeof(char));
    
    // Write a magic number, number of blocks and number of inodes to buffer
    memcpy(buffer, sb, sizeof(struct superblock));
    write_to_disk(0, buffer);
    free(buffer);   // free the buffer

    // -- free block vector 0-9 setting to unavailable --
    for (int i = 0; i < 9; i++){
        bitmapSet(free_block_vector, i);
    }

    // -- Block 1 -- 

    // Create a buffer
    buffer = (char *) calloc(BLOCK_SIZE, sizeof(char));

    memcpy(buffer, free_block_vector, BLOCK_SIZE*sizeof(uint8_t));
    write_to_disk(1, buffer);
    free(buffer);

    // -- Create Root Directory

}

int main(){
    InitLLFS();
    return 0;
}