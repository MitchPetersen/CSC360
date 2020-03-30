#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk_controller.h"

#define BLOCK_SIZE 512
#define NUM_BLOCKS 4096
#define vdisk_path "../disk/vdisk"

static int nblocks=4096;
static int nreads=0;
static int nwrites=0;

FILE* open_file(char* mode){
	FILE* disk = fopen(vdisk_path, mode);
	if (disk == NULL){
		printf("Error! Could not open file\n");
		exit(-1);
	}
	return disk;
}

void create_vdisk(){
	FILE* disk = open_file("wb+");

	// Initialize the disk with all zeros with size 512*4096 (0x200000) -> disk size
	char* buffer = (char *) calloc(BLOCK_SIZE*NUM_BLOCKS, sizeof(char));
	if (fwrite(buffer, sizeof(char), BLOCK_SIZE*NUM_BLOCKS, disk) != BLOCK_SIZE*NUM_BLOCKS){
		printf("Problems creating vdisk!");
		exit(1);
	}
	free(buffer);
	fclose(disk);
}

static void check_integrity(int block_index, const void *buffer){
	if(block_index < 0){
		printf("ERROR: block_index (%d) is negative!\n",block_index);
		exit(1);
	}

	if(block_index >= nblocks) {
		printf("ERROR: block_index (%d) is too big!\n",block_index);
		exit(1);
	}

	if(!buffer){
		printf("ERROR: null data pointer!\n");
		exit(1);
	}
}

// writing a block to disk
void write_to_disk(int block_index, char* buffer){

	check_integrity(block_index, buffer);

	FILE* disk = open_file("rb+");

	size_t seek = fseek(disk, BLOCK_SIZE*block_index, SEEK_SET);	// Reposition stream position indicator
	if(seek != 0){
		printf("Could not seek!");
		exit(1);
	}

	// Write the buffer to vdisk
	size_t result = fwrite(buffer, sizeof(char), BLOCK_SIZE, disk);
	if(result != BLOCK_SIZE){
		printf("Error writing to vdisk.");
		exit(1);
	}
	fclose(disk);
}

// reading a block from disk
char* read_from_disk(int block_index, char* buffer){

	check_integrity(block_index, buffer);

	FILE* disk;
	size_t seek;
	long size;
	size_t result;


	disk = open_file("rb");

	// obtain file size
	seek = fseek(disk, BLOCK_SIZE*block_index + BLOCK_SIZE, SEEK_SET/*BLOCK_SIZE*block_index*/);	// Reposition stream position indicator
	if(seek != 0){
		printf("Could not seek!");
		exit(1);
	}
	size = ftell(disk);
	rewind(disk);	// Set position of stream to the beginning

	// allocate memory to contain the whole file
	buffer = (char*) malloc (sizeof(char)*size);
	if (buffer == NULL){
		printf("Problems mallocing!");
		exit(1);
	}

	// copy the file into the buffer
	result = fread(buffer, 1, size, disk);
	if (result != size) {
		printf("Reading error!");
		exit(1);
	}
	fclose(disk);
	// TODO: DONT FORGET TO FREE BUFFER!!!
	return buffer;
}

