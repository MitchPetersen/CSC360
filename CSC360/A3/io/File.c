#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

const int BLOCK_SIZE = 512;
const int NUM_BLOCKS = 4096;
const int INODE_SIZE = 32;

int NUM_INODE = 0;
int ROOT_INODE_INDEX = -1;
int MAGIC = 1;
int MAPPING_BLOCK_START = 9; // block 9-24 for inode #1 - #2048
int x = 1;


void writeBlock(FILE* disk, int blockNum, char* data, int size){
    fseek(disk, blockNum * BLOCK_SIZE, SEEK_SET);
    fwrite(data, size, 1, disk); 
}


void readBlock(FILE* disk, int blockNum, char* buffer, int size){
    fseek(disk, blockNum * BLOCK_SIZE, SEEK_SET);
    fread(buffer, size, 1, disk);
}


void createSuperblock(FILE* disk){
	char* buffer = (char*)calloc(BLOCK_SIZE, 1);
	memcpy(buffer, &MAGIC, 4);
	memcpy(buffer+4, &NUM_BLOCKS, 4);
	memcpy(buffer+8, &NUM_INODE, 4);
	memcpy(buffer+12, &ROOT_INODE_INDEX, 4);
	memcpy(buffer+20, "NONE", 60);
	writeBlock(disk, 0, buffer, BLOCK_SIZE);
	free(buffer);
}


void readSuperblock(FILE* disk){ // reads block 0
	char* buffer = (char*)calloc(BLOCK_SIZE, 1);
    readBlock(disk, 0, buffer, BLOCK_SIZE);	
	memcpy(&NUM_INODE, buffer+8, 4);
	memcpy(&ROOT_INODE_INDEX, buffer+12, 4);
	free(buffer);
}


int getRootInodeIndex(FILE* disk){
	char* buffer = (char*)calloc(BLOCK_SIZE, 1);
    readBlock(disk, 0, buffer, BLOCK_SIZE);	
	int temp;
	memcpy(&temp, buffer+12, 4);
	free(buffer);
	return temp;
}


void updateSuperblock(FILE* disk){
	char* buffer = (char*)calloc(BLOCK_SIZE, 1);
    readBlock(disk, 0, buffer, BLOCK_SIZE);
	memcpy(buffer+4, &NUM_BLOCKS, 4);
	memcpy(buffer+8, &NUM_INODE, 4);
	memcpy(buffer+12, &ROOT_INODE_INDEX, 4);
	writeBlock(disk, 0, buffer, BLOCK_SIZE);
	free(buffer);
}


void robustSuperblock(FILE* disk, char* input, int contentLength){
	char* buffer = (char*)calloc(BLOCK_SIZE, 1);
    readBlock(disk, 0, buffer, BLOCK_SIZE);
	memcpy(buffer+16, &contentLength, 4);
	memcpy(buffer+20, input, 60);
	writeBlock(disk, 0, buffer, BLOCK_SIZE);	
	free(buffer);
}


void createFreeBlockVector(FILE* disk){
	for(int j = 1; j < 9; j++) {
		if(j == 1){
			char freeBlockContent[BLOCK_SIZE];
			for(int i = 0; i < BLOCK_SIZE; i++){
				if(i < 25){
					freeBlockContent[i] = '0';
				} else {
					freeBlockContent[i] = '1';
				}
			}
			writeBlock(disk, 1, freeBlockContent, BLOCK_SIZE);
		} else {
			char freeBlockContent2[BLOCK_SIZE];
			for(int i = 0; i < BLOCK_SIZE; i++){
				freeBlockContent2[i] = '1';
			}
			writeBlock(disk, j, freeBlockContent2, BLOCK_SIZE);
		}
	}
}


void readFreeBlockVector(FILE* disk, int* findFreeBlock){
	int signal = 0;
	int freeBlockIndex = 0;
	
	for(int i = 1; (i < 9) && (signal == 0); i++){	
		char* buffer = (char*)calloc(BLOCK_SIZE, 1);
		readBlock(disk, i, buffer, BLOCK_SIZE);
		for(int j=0; (j < BLOCK_SIZE) && (signal == 0); j++){
			if(buffer[j] == '1'){
				signal = 1;
				freeBlockIndex = j +((i-1)*512);
			}			
		}
		free(buffer);	
	}
	memcpy(findFreeBlock, &freeBlockIndex, 4);
}


void fillFreeBlockVector(FILE* disk, int fillBlockNumber){
	int freeBlock = fillBlockNumber / 128; 
	int remainder = fillBlockNumber % 128;
	char* buffer = (char*)calloc(BLOCK_SIZE, 1);	

	readBlock(disk, freeBlock+1, buffer, BLOCK_SIZE);
	buffer[remainder] = '0';
	writeBlock(disk, freeBlock+1, buffer, BLOCK_SIZE);
	free(buffer);		
}


void deleteFreeBlockVector(FILE* disk, int deleteBlock){
	int freeBlock = deleteBlock / 128; 
	int remainder = deleteBlock % 128;
	char* buffer = (char*)calloc(BLOCK_SIZE, 1);	

	readBlock(disk, freeBlock+1, buffer, BLOCK_SIZE);
	buffer[remainder] = '1';
	writeBlock(disk, freeBlock+1, buffer, BLOCK_SIZE);
	free(buffer);	
}


void addMapping(FILE* disk, int inodeNumber, int inodeBlock){	// assume max inode=2048, 2048*4/512=16
	int temp = (inodeNumber-1) / 128;
	int mappingBlock = MAPPING_BLOCK_START + temp;
	char* buffer = (char*)calloc(BLOCK_SIZE, 1);

	readBlock(disk, mappingBlock, buffer, BLOCK_SIZE);
	memcpy(buffer +((inodeNumber-1) * 4), &inodeBlock, 4); 
	writeBlock(disk, mappingBlock, buffer, BLOCK_SIZE);
	free(buffer);	
}


void findNextFreeInodenum(FILE* disk, int* inodeNumber){
	int signal = 0;
	int freeInodeIndex = 0
	for(int i = 9;(i < 24)&&(signal == 0); i++){	
		char* buffer = (char*)calloc(BLOCK_SIZE, 1);
		readBlock(disk, i, buffer, BLOCK_SIZE);
		for(int j=0;(j < 128)&&(signal == 0); j++){
			int temp;
			memcpy(&temp,(buffer +(j*4)), 4); 
			if(temp == 0){
				signal = 1;
				freeInodeIndex = j +((i-9)*128) + 1;
			}			
		}
		free(buffer);	
	}
	memcpy(inodeNumber, &freeInodeIndex, 4);
}


int findMapping(FILE* disk, int inodeNumber){
	int temp = (inodeNumber-1) / 128; 
	int mapping_block = MAPPING_BLOCK_START + temp;
	char* buffer = (char*)calloc(BLOCK_SIZE, 1);
	
	readBlock(disk, mapping_block, buffer, BLOCK_SIZE);
	
	int temp2;
	memcpy(&temp2, buffer +((inodeNumber-1) * 4), 4); 
	free(buffer);	
	return temp_block_num;
}


void deleteMapping(FILE* disk, int inodeNumber){
	int temp = (inodeNumber-1) / 128; 
	int mappingBlock = MAPPING_BLOCK_START + temp;
	char* buffer = (char*)calloc(BLOCK_SIZE, 1);
	
	readBlock(disk, mappingBlock, buffer6, BLOCK_SIZE);
	
	int clearInode = 0;
	memcpy(buffer + ((inodeNumber-1) * 4), &clearInode, 4); 
	writeBlock(disk, mappingBlock, buffer, BLOCK_SIZE);
	free(buffer);	
}


void createSingleIndirect(FILE* disk, short* fileBlockArray, int arraySize, short* singleIndirectBlockNum){
	int freeBlockNum;
	readFreeBlockVector(disk, &freeBlockNum);
	fillFreeBlockVector(disk, freeBlockNum);	
	char* singleIndirectContent = (char*)calloc(BLOCK_SIZE, 1);
	
	for(int i = 10; i < arraySize; i++){
		memcpy(single_indir_content + ((i-10)*2), &fileBlockArray[i], 2);
	}

	writeBlock(disk, freeBlockNum, single_indir_content, BLOCK_SIZE);
	short temp	= (short) freeBlockNum; 
	memcpy(singleIndirectBlockNum, &temp, 2);
}


void IndirectToSize(FILE* disk, short indirectBlockNum, short* resultSize){		
	char* buffer = (char*)calloc(BLOCK_SIZE, 1);
    readBlock(disk, indirectBlockNum, buffer, BLOCK_SIZE);
	short tempSize = 0;
	short tempBlockNum = 1;
	
	for(int i = 0; i < (BLOCK_SIZE/2) && (tempBlockNum!= 0); i++){
		memcpy(&tempBlockNum, buffer +(i*2), 2);
		if(tempBlockNum > 0){
			tempSize++;
		}
	}
	tempSize = tempSize + 10;
	memcpy(resultSize, &tempSize, 2);
}


void IndirectToArray(FILE* disk, short indirect_block_num, short* result_block_num, short array_size){		
	char* buffer = (char*)calloc(BLOCK_SIZE, 1);
    readBlock(disk, indirect_block_num, buffer, BLOCK_SIZE);
	
	for(int i = 10; i < array_size; i++){
		memcpy(&result_block_num[i], buffer +((i-10)*2), 2);
	}
}


void createFileInode(FILE* disk, int freeBlock, short* fileBlockNumberArray, int arraySize, int fileSize){
	char* inode = calloc(BLOCK_SIZE, 1);
	// 1 = file, 2 = directory
	int flags = 1;
	short dataBlock[12];
	
	for(int i = 0; i < 12; i++){
		dataBlock[i] = -1;
		memcpy(inode + 8 + (i*2), &dataBlock[i], 2);
	}
	
	if(arraySize < 11){
		for(int i = 0; i < arraySize; i++){
			dataBlock[i] = fileBlockNumberArray[i];
			memcpy(inode+8+(i*2), &dataBlock[i], 2);
		}
		memcpy(inode, &fileSize, 4);
		memcpy(inode+4, &flags, 4);
		
		short single_indirect = -1; 
		short double_indirect = -1; 
		
		memcpy(inode+28, &single_indirect, 2);
		memcpy(inode+30, &double_indirect, 2);	
		
		writeBlock(disk, freeBlock, inode, BLOCK_SIZE);
	} else {
		for(int i = 0; i < 10; i++){
			dataBlock[i] = fileBlockNumberArray[i];
			memcpy(inode+8+(i*2), &dataBlock[i], 2);
		}
		short single_indirect_block_num = -1;
		createSingleIndirect(disk, fileBlockNumberArray, arraySize, &single_indirect_block_num);
		
		memcpy(inode, &fileSize, 4);
		memcpy(inode+4, &flags, 4);
		
		short single_indirect = single_indirect_block_num; 
		short double_indirect = -1; 
		
		memcpy(inode+28, &single_indirect, 2);
		memcpy(inode+30, &double_indirect, 2);	
		writeBlock(disk, freeBlock, inode, BLOCK_SIZE);
	}
	free(inode);
}


void create_file_single_inode(FILE* disk, int free_block_num, int dir_block_num){

	char* inode = calloc(BLOCK_SIZE, 1);
	int size_of_file = 0;
	int flags = 1;    							// 1 = file, 2 = directory
	short dataBlock[12];
	
	for(int i = 0; i < 12; i++){				// initialize each dataBlock[] = -1
		dataBlock[i] = -1;
		memcpy(inode+8+(i*2), &dataBlock[i], 2);
	}

    short dataBlock_0	= (short) dir_block_num;       // the block for this directory
	
	memcpy(inode, &size_of_file, 4);
	memcpy(inode+4, &flags, 4);
	memcpy(inode+8, &dataBlock_0, 2);

	writeBlock(disk, free_block_num, inode, BLOCK_SIZE);
	
	free(inode);
}	
	
void create_directory_inode(FILE* disk, int free_block_num, int dir_block_num){

	char* inode = calloc(BLOCK_SIZE, 1);
	int size_of_file = 0;
	int flags = 2;    							// 1 = file, 2 = directory
	short dataBlock[12];
	
	for(int i = 0; i < 12; i++){				// initialize each dataBlock[] = -1
		dataBlock[i] = -1;
		memcpy(inode+8+(i*2), &dataBlock[i], 2);
	}

    short dataBlock_0	= (short) dir_block_num;       // the block for this directory
	
	memcpy(inode, &size_of_file, 4);
	memcpy(inode+4, &flags, 4);
	memcpy(inode+8, &dataBlock_0, 2);

	writeBlock(disk, free_block_num, inode, BLOCK_SIZE);
	
	free(inode);
}	

void read_inode(FILE* disk, int inode_block_num, short* result_block_num){		// we should get back an array of block num

	char* buffer = (char*)calloc(32, 1);
    readBlock(disk, inode_block_num, buffer, 32);

	for(int i = 0; i < 12; i++){

		memcpy(&result_block_num[i], buffer +(8 + 2*i), 2);
	}
}

void create_directory_block(FILE* disk, int free_block_num2){
	
	char* data = (char*)calloc(BLOCK_SIZE, 1);
	//char file_name[] = "file_name_testing";
	//unsigned char inode_number;
	//memcpy(data, &inode_number, 1);
	//strncpy(data+1, name, 31);

	writeBlock(disk, free_block_num2, data, BLOCK_SIZE);

	free(data);
}

void extend_parent_dir_block(FILE* disk, int parent_dir_node_block_num, int* new_block_num){
	
	int free_block_num3;
	
	readFreeBlockVector(disk, &free_block_num3);
	create_directory_block(disk, free_block_num3);
	fillFreeBlockVector(disk, free_block_num3);
	
	memcpy(new_block_num, &free_block_num3, 4);

	int latest_datablock;
	short result_block_num[12];
	
	read_inode(disk, parent_dir_node_block_num, result_block_num);

	for(int i = 0; i < 10; i++){

		if(result_block_num[i] != -1){
		
			latest_datablock = i;
		}
	}
	
	result_block_num[latest_datablock+1] = (short) free_block_num3;

	char* inode = calloc(32, 1);
    readBlock(disk, parent_dir_node_block_num, inode, 32);				
	memcpy(inode+8+((latest_datablock+1)*2), &result_block_num[latest_datablock+1], 2);
	writeBlock(disk, parent_dir_node_block_num, inode, 32);

	free(inode);
}

void edit_parent_dir_block(FILE* disk, int parent_dir_node_block_num, int child_dir_inode_num, char* child_dir_name){
	
	int latest_datablock;
	short result_block_num[12];
	
	read_inode(disk, parent_dir_node_block_num, result_block_num);

	for(int i = 0; i < 10; i++){
		if(result_block_num[i] != -1){
			latest_datablock = i;
		}
	}
	//printf("\n\n\n\n   *****latest_datablock: %d   \n\n\n\n", latest_datablock);  
	
	char* buffer7 = (char*)calloc(BLOCK_SIZE, 1);	
	readBlock(disk, result_block_num[latest_datablock], buffer7, BLOCK_SIZE);  

	int entry_num = -1; 
	
	for(int i = 0;(i < 16)&&(entry_num == -1); i++){
		
		unsigned char temp_inode_number; 
		memcpy(&temp_inode_number, buffer7 +(i*32), 1); 
		
		int temp_inode_number2 = (int) temp_inode_number; 

		if(temp_inode_number2 == 0){
			entry_num = i;
		}
	}
	
	if(entry_num == -1){			// full entry
		
		printf("\n\n\n\n   *****extending directory block of parent   \n\n\n\n");
		
		int new_block_num;
		extend_parent_dir_block(disk, parent_dir_node_block_num, &new_block_num);	// create new block+link it up to parent inode
		
		unsigned char inode_number = (char) child_dir_inode_num;

		memcpy(buffer7 +(entry_num*32), &inode_number, 1);			//child_dir_inode_num = new build dir inode#
		strncpy(buffer7 +(entry_num*32) +1, child_dir_name, 31);

		writeBlock(disk, new_block_num, buffer7, BLOCK_SIZE);
	}
	
	else{
		unsigned char inode_number = (char) child_dir_inode_num;

		memcpy(buffer7 +(entry_num*32), &inode_number, 1);
		strncpy(buffer7 +(entry_num*32) +1, child_dir_name, 31);

		writeBlock(disk, result_block_num[latest_datablock], buffer7, BLOCK_SIZE);
	}
	free(buffer7);
}




void del_entry_dir_block(FILE* disk, int parent_dir_block_num, int del_file_inode_num){
	
	char* buffer7 = (char*)calloc(BLOCK_SIZE, 1);	
	readBlock(disk, parent_dir_block_num, buffer7, BLOCK_SIZE);

	int entry_num = -1;
	
	for(int i = 0;(i < 16)&&(entry_num == -1); i++){
		
		unsigned char temp_inode_number;
		memcpy(&temp_inode_number, buffer7 +(i*32), 1);
		
		int temp_inode_number2 = (int) temp_inode_number;
		//printf("*****testing: %d\n", temp_inode_number2);
		
		if(temp_inode_number2 == del_file_inode_num){
			entry_num = i;
		}
	}
	//printf("*****testing deleteing_entry_num: %d\n", entry_num);

	char* temp_entry_buffer = (char*)calloc(32, 1);	
	memcpy(buffer7 +(entry_num*32), temp_entry_buffer, 32);
	writeBlock(disk, parent_dir_block_num, buffer7, BLOCK_SIZE);
	
	free(buffer7);
}



void search_file_or_dir(FILE* disk, int dir_block_num, char* file_name, int* finding_inode_num){

	int entry_num = -1;

	char* buffer3 = (char*)calloc(BLOCK_SIZE, 1);
	readBlock(disk, dir_block_num, buffer3, BLOCK_SIZE);
	
	for(int i = 0;(i < 16)&&(entry_num == -1); i++){

		char* buffer4 = (char*)calloc(31, 1);	
		strncpy(buffer4, buffer3 +(i*32) + 1, 30);

		if(strncmp(buffer4, file_name, 30) == 0){
			
			entry_num = i;
			//printf("@@@@@@@@@ matching entry_num: %d\n", entry_num);
		}
		free(buffer4); 
	}
	
	if(entry_num != -1){
		
		unsigned char temp_inode_num_hello;
		memcpy(&temp_inode_num_hello, buffer3 +(entry_num*32), 1);

		int temp_inode_num_hello2 = (int) temp_inode_num_hello;
		//printf("***********checking for inode#: %d\n", temp_inode_num_hello2);
		memcpy(finding_inode_num, &temp_inode_num_hello2, 4);
	
	} else {
		
		int temp_inode_num_hello2 = -1;
		//printf("\n\n      no such directory or file in this datablock: '%s'   \n\n", file_name);
		memcpy(finding_inode_num, &temp_inode_num_hello2, 4);
	}
	
	free(buffer3);
	
}


void create_file_block(FILE* disk, int free_block_num2, char* file_content_buffer){

	writeBlock(disk, free_block_num2, file_content_buffer, BLOCK_SIZE);

}


void create_root(FILE* disk){
	
	int free_block_num2;
	readFreeBlockVector(disk, &free_block_num2);
	fillFreeBlockVector(disk, free_block_num2);

	int root_dir_block_num = free_block_num2;     
	int free_block_num;
	readFreeBlockVector(disk, &free_block_num);
	create_directory_inode(disk, free_block_num, root_dir_block_num);
	fillFreeBlockVector(disk, free_block_num);
	
	int next_free_inode_index2;
	findNextFreeInodenum(disk, &next_free_inode_index2);
	//printf("***********************testing find_next_free_inode #root: %d\n", next_free_inode_index2);
	
	addMapping(disk, next_free_inode_index2, free_block_num);	// root dir inode = #1, (#01, block #25)
	
	ROOT_INODE_INDEX = next_free_inode_index2;
	update_superblock(disk);
}


void initLLFS(FILE* disk){
	
	//char* init = calloc(BLOCK_SIZE*NUM_BLOCKS, 1);
	//fwrite(init, BLOCK_SIZE*NUM_BLOCKS, 1, disk);
	//free(init);
	
	createSuperblock(disk);
	createFreeBlockVector(disk);
	create_root(disk);

	NUM_INODE++;
	update_superblock(disk);
	//readSuperblock(disk);
}


void create_sub_directory(FILE* disk, int parent_dir_node_block_num, char* child_dir_name){
	
	int free_block_num3;
	readFreeBlockVector(disk, &free_block_num3);
	create_directory_block(disk, free_block_num3);
	fillFreeBlockVector(disk, free_block_num3);

	int sub_dir_block_num = free_block_num3;    
	int free_block_num4; 
	readFreeBlockVector(disk, &free_block_num4);
	create_directory_inode(disk, free_block_num4, sub_dir_block_num);
	fillFreeBlockVector(disk, free_block_num4);
	
	int next_free_inode_index3;
	findNextFreeInodenum(disk, &next_free_inode_index3);
	//printf("testing find_next_free_inode #sub: %d\n", next_free_inode_index3);

	addMapping(disk, next_free_inode_index3, free_block_num4);		// sub dir inode = #2, (#02, block #28)
	
	NUM_INODE++;
	update_superblock(disk);
	//readSuperblock(disk);

	
	//modify parent(root)
	int child_dir_inode_num = next_free_inode_index3;   
	edit_parent_dir_block(disk, parent_dir_node_block_num, child_dir_inode_num, child_dir_name);
}



void create_file(FILE* disk, char* file_content, int parent_dir_inode_num, char* file_name){
	
	//printf("\n\n &&&&&&&&&&&&&&&&&&&&&& testing length of file_content: %d\n", strlen(file_content));

	if(strlen(file_content) < 512){

		int free_block_num4;											// block for file content
		char* file_content_buffer = (char*)calloc(BLOCK_SIZE, 1);
		
		strncpy(file_content_buffer, file_content, strlen(file_content));
		//printf("testing file_content: %s\n", file_content_buffer);
		
		readFreeBlockVector(disk, &free_block_num4);
		create_file_block(disk, free_block_num4, file_content_buffer);
		fillFreeBlockVector(disk, free_block_num4);


		int file_block_num = free_block_num4;   
		int free_block_num5;											// block for file inode

		readFreeBlockVector(disk, &free_block_num5);
		create_file_single_inode(disk, free_block_num5, file_block_num);
		fillFreeBlockVector(disk, free_block_num5);

		
		int next_free_inode_index4;
		
		findNextFreeInodenum(disk, &next_free_inode_index4);
		addMapping(disk, next_free_inode_index4, free_block_num5);	// file inode = #3
		
		
		NUM_INODE++;
		update_superblock(disk);										//update superblock info
		//readSuperblock(disk);
		
		
		//add to parent inode(into sub directory)

		int file_inode_num = next_free_inode_index4;   
		edit_parent_dir_block(disk, parent_dir_inode_num, file_inode_num, file_name);
	}
	else {
		
		int block_needed;
		
		if((strlen(file_content) % 512) != 0){						// case: if doesnt fill whole block
			block_needed = (strlen(file_content))/512+1;
		}else{
			block_needed = (strlen(file_content))/512;
		}
			
		short store_block_array[ block_needed ];
		//printf("\n\n testing block_needed : %d \n\n\n", block_needed);
			
		for(int i = 0; i < block_needed; i++){

			int free_block_num4;									// block for file content
			
			char* file_content_buffer = (char*)calloc(BLOCK_SIZE, 1);
			strncpy(file_content_buffer, file_content +(i*512), 512);

			readFreeBlockVector(disk, &free_block_num4);
			create_file_block(disk, free_block_num4, file_content_buffer);
			fillFreeBlockVector(disk, free_block_num4);

			store_block_array[i] = (short) free_block_num4;

			//printf("\ntesting store_block_array %d: %d\n", i,  store_block_array[i]);
		}

		int size_of_array = block_needed;
		
		int free_block_num5;			// block for file inode
		int size_of_file = strlen(file_content);

		readFreeBlockVector(disk, &free_block_num5);
		fillFreeBlockVector(disk, free_block_num5);
		createFileInode(disk, free_block_num5, store_block_array, size_of_array, size_of_file);


		int next_free_inode_index4;
		
		findNextFreeInodenum(disk, &next_free_inode_index4);
		addMapping(disk, next_free_inode_index4, free_block_num5);	// file inode = #3

		
		NUM_INODE++;
		update_superblock(disk);		//update superblock info
		//readSuperblock(disk);
		
		
		//add to parent inode(into sub directory)

		int file_inode_num = next_free_inode_index4;   
		edit_parent_dir_block(disk, parent_dir_inode_num, file_inode_num, file_name);
	}
}



void create_empty_file(FILE* disk, int parent_dir_inode_num, char* file_name){

		int free_block_num4;											// block for file content
		char* file_content_buffer = (char*)calloc(BLOCK_SIZE, 1);
		
		readFreeBlockVector(disk, &free_block_num4);
		create_file_block(disk, free_block_num4, file_content_buffer);
		fillFreeBlockVector(disk, free_block_num4);


		int file_block_num = free_block_num4;   
		int free_block_num5;											// block for file inode

		readFreeBlockVector(disk, &free_block_num5);
		create_directory_inode(disk, free_block_num5, file_block_num);
		fillFreeBlockVector(disk, free_block_num5);

		
		int next_free_inode_index4;
		
		findNextFreeInodenum(disk, &next_free_inode_index4);
		addMapping(disk, next_free_inode_index4, free_block_num5);	// file inode = #3
		
		
		NUM_INODE++;
		update_superblock(disk);										//update superblock info
		//readSuperblock(disk);
		
		
		//add to parent inode(into sub directory)

		int file_inode_num = next_free_inode_index4;   
		edit_parent_dir_block(disk, parent_dir_inode_num, file_inode_num, file_name);
}



void open_file(FILE* disk, char* input){

	char* parent_directory_name = (char*)calloc(31,1);	
	char* curr_file_name = (char*)calloc(31,1);	
	char* fake_curr_file_name = (char*)calloc(31,1);	
	
	const char s[2] = "/";

	fake_curr_file_name = strtok(input, s);		// skip the first command "open"
		
	strncpy(parent_directory_name, curr_file_name, 31);
	strncpy(curr_file_name, fake_curr_file_name, 31);
	
	fake_curr_file_name = strtok(NULL, s);
	
	int saved_parent_inode_block_num = -1;
	
		int newest_ROOT_INODE_INDEX = getRootInodeIndex(disk);
			
		int root_inode_block_num = findMapping(disk, newest_ROOT_INODE_INDEX);

		short result_block_num4[12];
		read_inode(disk, root_inode_block_num, result_block_num4);
			
		saved_parent_inode_block_num = root_inode_block_num;	

	
	while(fake_curr_file_name != NULL) {

		strncpy(parent_directory_name, curr_file_name, 31);
		strncpy(curr_file_name, fake_curr_file_name, 31);
		fake_curr_file_name = strtok(NULL, s);
		//printf("curr_file_name: %s ", curr_file_name);
		//printf("parent_directory_name: %s\n", parent_directory_name);

		if(fake_curr_file_name == NULL){
			
			int checking_inode_num;		
			short result_block_num3[12];
	
			read_inode(disk, saved_parent_inode_block_num, result_block_num3);
			
			for(int i = 0; result_block_num3[i] != -1; i++){
			
				search_file_or_dir(disk, result_block_num3[i], curr_file_name, &checking_inode_num);
				
				if(checking_inode_num > 0){
					
					saved_parent_inode_block_num = findMapping(disk, checking_inode_num);
				}
			}

			if(checking_inode_num < 1){
				printf("\n      no such directory or file in this datablock: '%s'   \n", curr_file_name);
				break;
			}
			
			short result_block_num[12];
			read_inode(disk, saved_parent_inode_block_num, result_block_num);		// saved_parent=file_inode
			
			if(result_block_num[10] > 0){
				
				//printf("\n\n testing result_block_num[10] : %d \n\n\n", result_block_num[10]);
				
				short result_size;
				IndirectToSize(disk, result_block_num[10], &result_size);
				
				//printf("\n\n testing result_size : %d \n\n\n", result_size);
				
				short result_block_num5[result_size];
				read_inode(disk, saved_parent_inode_block_num, result_block_num5);	
				
				//printf("\n\n testing result_block_num5[10] : %d \n\n\n", result_block_num5[10]);
				
				translate_indirect_to_array(disk, result_block_num[10], result_block_num5, result_size);
				
				//printf("\n\n testing result_block_num5[10] : %d \n\n\n", result_block_num5[10]);
				
				printf("\n\n       content of the file: \n");
				
				for(int i = 0;(result_block_num5[i] != -1) &&(i<result_size); i++){
					
					char* buffer = (char*)calloc(BLOCK_SIZE,1);	
					readBlock(disk,(int)result_block_num5[i], buffer, BLOCK_SIZE);
					printf("%s", buffer);
						
					free(buffer);
				}
				printf("\n\n\n");
			}
			else{
				printf("\n\n       content of the file: \n");
				
				for(int i = 0;(result_block_num[i] != -1) &&(i<10); i++){
					
					char* buffer = (char*)calloc(BLOCK_SIZE,1);	
					readBlock(disk,(int)result_block_num[i], buffer, BLOCK_SIZE);
					printf("%s", buffer);
						
					free(buffer);
				}
				printf("\n\n\n");
			}
		}

		else {
		
			int checking_inode_num;		
			short result_block_num[12];
	
			read_inode(disk, saved_parent_inode_block_num, result_block_num);
			
			for(int i = 0; result_block_num[i] != -1; i++){
			
				search_file_or_dir(disk, result_block_num[i], curr_file_name, &checking_inode_num);
				
				if(checking_inode_num > 0){
					
					saved_parent_inode_block_num = findMapping(disk, checking_inode_num);
				}
			}
			
			if(checking_inode_num < 1){
				printf("\n      no such directory in this datablock: '%s'   \n", curr_file_name);
				break;
			}
		}
	}
	
	free(parent_directory_name);
	free(curr_file_name);
	free(fake_curr_file_name);	
}

void make_directory(FILE* disk, char* input){
	
	char* parent_directory_name = (char*)calloc(31,1);	
	char* curr_dir_name = (char*)calloc(31,1);	
	char* fake_curr_dir_name = (char*)calloc(31,1);	
	
	const char s[2] = "/";

	fake_curr_dir_name = strtok(input, s);		// skip "Mkdir"

	strncpy(parent_directory_name, curr_dir_name, 31);
	strncpy(curr_dir_name, fake_curr_dir_name, 31);
	
	fake_curr_dir_name = strtok(NULL, s);
	
	int saved_parent_inode_block_num = -1;
	
	int newest_ROOT_INODE_INDEX = getRootInodeIndex(disk);
		
	int root_inode_block_num = findMapping(disk, newest_ROOT_INODE_INDEX);

	short result_block_num4[12];
	read_inode(disk, root_inode_block_num, result_block_num4);
		
	saved_parent_inode_block_num = root_inode_block_num;
	
	while(fake_curr_dir_name != NULL) {

		strncpy(parent_directory_name, curr_dir_name, 31);
		strncpy(curr_dir_name, fake_curr_dir_name, 31);
		fake_curr_dir_name = strtok(NULL, s);
		printf("curr_dir_name: %s ", curr_dir_name);
		printf("parent_directory_name: %s\n", parent_directory_name);

		if(fake_curr_dir_name == NULL){

			create_sub_directory(disk, saved_parent_inode_block_num, curr_dir_name);

			printf("\n       $$$$$$$$$$$$$$ we added a sub directory: %s\n\n", curr_dir_name);
			
		}
		else{

			int checking_inode_num;		
			short result_block_num[12];
	
			read_inode(disk, saved_parent_inode_block_num, result_block_num);
			
			for(int i = 0; result_block_num[i] != -1; i++){
			
				search_file_or_dir(disk, result_block_num[i], curr_dir_name, &checking_inode_num);
				
				if(checking_inode_num > 0){
					
					saved_parent_inode_block_num = findMapping(disk, checking_inode_num);
				}
			}
			
			if(checking_inode_num < 1){
				printf("\n      no such directory in this datablock: '%s'   \n", curr_dir_name);
				break;
			}
		}
	}
	free(parent_directory_name);
	free(curr_dir_name);
	free(fake_curr_dir_name);	
}




void write_file(FILE* disk, char* input, char* file_content_larger){
	
	char* parent_directory_name = (char*)calloc(31,1);	
	char* curr_file_name = (char*)calloc(31,1);	
	char* fake_curr_dir_name = (char*)calloc(31,1);	
	
	const char s[2] = "/";

	fake_curr_dir_name = strtok(input, s);		// skip the first command "Mkfile"

	strncpy(parent_directory_name, curr_file_name, 31);
	strncpy(curr_file_name, fake_curr_dir_name, 31);
	
	fake_curr_dir_name = strtok(NULL, s);
	
	int saved_parent_inode_block_num = -1;
	
	int newest_ROOT_INODE_INDEX = getRootInodeIndex(disk);
		
	int root_inode_block_num = findMapping(disk, newest_ROOT_INODE_INDEX);

	short result_block_num4[12];
	
	read_inode(disk, root_inode_block_num, result_block_num4);
		
	saved_parent_inode_block_num = root_inode_block_num;
	
	while(fake_curr_dir_name != NULL) {

		strncpy(parent_directory_name, curr_file_name, 31);
		strncpy(curr_file_name, fake_curr_dir_name, 31);
		fake_curr_dir_name = strtok(NULL, s);
		//printf("curr_file_name: %s ", curr_file_name);
		//printf("parent_directory_name: %s\n", parent_directory_name);

		if(fake_curr_dir_name == NULL){
			
			create_file(disk, file_content_larger, saved_parent_inode_block_num, curr_file_name);

			printf("\n       $$$$$$$$$$$$$$ we added a file, file name: %s\n\n", curr_file_name);
		}
		else{

			int checking_inode_num;		
			short result_block_num[12];
	
			read_inode(disk, saved_parent_inode_block_num, result_block_num);
			
			for(int i = 0; result_block_num[i] != -1; i++){
			
				search_file_or_dir(disk, result_block_num[i], curr_file_name, &checking_inode_num);
				
				if(checking_inode_num > 0){
					
					saved_parent_inode_block_num = findMapping(disk, checking_inode_num);
				}
			}
			
			if(checking_inode_num < 1){
				printf("\n      no such directory in this datablock: '%s'   \n", curr_file_name);
				break;
			}
		}
	}
	free(parent_directory_name);
	free(curr_file_name);
	free(fake_curr_dir_name);	
}



void write_empty_file(FILE* disk, char* input){
	
	char* parent_directory_name = (char*)calloc(31,1);	
	char* curr_file_name = (char*)calloc(31,1);	
	char* fake_curr_dir_name = (char*)calloc(31,1);	
	
	const char s[2] = "/";

	fake_curr_dir_name = strtok(input, s);		// skip the first command "Mkfile"

	strncpy(parent_directory_name, curr_file_name, 31);
	strncpy(curr_file_name, fake_curr_dir_name, 31);
	
	fake_curr_dir_name = strtok(NULL, s);
	
	int saved_parent_inode_block_num = -1;
	
		int newest_ROOT_INODE_INDEX = getRootInodeIndex(disk);
			
		int root_inode_block_num = findMapping(disk, newest_ROOT_INODE_INDEX);

		short result_block_num4[12];
		read_inode(disk, root_inode_block_num, result_block_num4);
			
		saved_parent_inode_block_num = root_inode_block_num;
	
	while(fake_curr_dir_name != NULL) {

		strncpy(parent_directory_name, curr_file_name, 31);
		strncpy(curr_file_name, fake_curr_dir_name, 31);
		fake_curr_dir_name = strtok(NULL, s);
		//printf("curr_file_name: %s ", curr_file_name);
		//printf("parent_directory_name: %s\n", parent_directory_name);

		if(fake_curr_dir_name == NULL){
			
			create_empty_file(disk, saved_parent_inode_block_num, curr_file_name);

			printf("\n       $$$$$$$$$$$$$$ we added an empty file, file name: %s\n\n", curr_file_name);
		}
		else{

			int checking_inode_num;		
			short result_block_num[12];
	
			read_inode(disk, saved_parent_inode_block_num, result_block_num);
			
			for(int i = 0; result_block_num[i] != -1; i++){
			
				search_file_or_dir(disk, result_block_num[i], curr_file_name, &checking_inode_num);
				
				if(checking_inode_num > 0){
					
					saved_parent_inode_block_num = findMapping(disk, checking_inode_num);
				}
			}
			
			if(checking_inode_num < 1){
				printf("\n      no such directory in this datablock: '%s'   \n", curr_file_name);
				break;
			}
		}
	}
	free(parent_directory_name);
	free(curr_file_name);
	free(fake_curr_dir_name);	
}


void delete_file(FILE* disk, int parent_dir_block_num, char* curr_file_name, int del_file_inode_num){

	// free file block + inode block
	// parent dir data block inside remove file
	// delete inode mapping	
	
	short del_block_num[12];
	int del_inode_block_num = findMapping(disk, del_file_inode_num);

	read_inode(disk, del_inode_block_num, del_block_num);

	for(int i = 0; del_block_num[i] != -1; i++){
		deleteFreeBlockVector(disk, del_block_num[i]);
	}
	
	deleteFreeBlockVector(disk, del_inode_block_num);
	NUM_INODE--;
	update_superblock(disk);
	
	deleteMapping(disk, del_file_inode_num);
	
	del_entry_dir_block(disk, parent_dir_block_num, del_file_inode_num);
}


void delete_directory(FILE* disk, int parent_dir_block_num, char* curr_file_name, int del_dir_inode_num){
	

	// free file block + inode block
	// parent dir data block inside remove file
	// delete inode mapping	
	
	short del_block_num[12];
	int del_inode_block_num = findMapping(disk, del_dir_inode_num);
	
	read_inode(disk, del_inode_block_num, del_block_num);
	
	for(int i = 0; del_block_num[i] != -1; i++){
		deleteFreeBlockVector(disk, del_block_num[i]);
	}
	
	deleteFreeBlockVector(disk, del_inode_block_num);
	NUM_INODE--;
	update_superblock(disk);
	
	deleteMapping(disk, del_dir_inode_num);
	
	del_entry_dir_block(disk, parent_dir_block_num, del_dir_inode_num);
}


void Rm_file(FILE* disk, char* input){
	
	char* parent_directory_name = (char*)calloc(31,1);	
	char* curr_file_name = (char*)calloc(31,1);	
	char* fake_curr_dir_name = (char*)calloc(31,1);	
	
	const char s[2] = "/";

	fake_curr_dir_name = strtok(input, s);		// skip the first command "Mkfile"

	strncpy(parent_directory_name, curr_file_name, 31);
	strncpy(curr_file_name, fake_curr_dir_name, 31);
	
	fake_curr_dir_name = strtok(NULL, s);
	
	int saved_parent_inode_block_num = -1;
	
		int newest_ROOT_INODE_INDEX = getRootInodeIndex(disk);
			
		int root_inode_block_num = findMapping(disk, newest_ROOT_INODE_INDEX);

		short result_block_num4[12];
		read_inode(disk, root_inode_block_num, result_block_num4);
			
		saved_parent_inode_block_num = root_inode_block_num;
	
	while(fake_curr_dir_name != NULL) {

		strncpy(parent_directory_name, curr_file_name, 31);
		strncpy(curr_file_name, fake_curr_dir_name, 31);
		fake_curr_dir_name = strtok(NULL, s);
		//printf("curr_file_name: %s ", curr_file_name);
		//printf("parent_directory_name: %s\n", parent_directory_name);

		if(fake_curr_dir_name == NULL){
			
			short result_block_num[12];
			read_inode(disk, saved_parent_inode_block_num, result_block_num);

			int checking_inode_num = -1;		
			int file_appear_in_which_block_of_directory;
			
			for(int i = 0;(result_block_num[i] != -1) &&(checking_inode_num == -1); i++){
			
				search_file_or_dir(disk, result_block_num[i], curr_file_name, &checking_inode_num);
				
				if(checking_inode_num != -1){
				
					file_appear_in_which_block_of_directory = i;
				}
			}
			
			if(checking_inode_num == -1){
				printf("file not exist\n");
				break;
			}
			
			delete_file(disk, result_block_num[file_appear_in_which_block_of_directory], curr_file_name, checking_inode_num);

			printf("\n       $$$$$$$$$$$$$$ we deleted a file, file name: %s\n\n", curr_file_name);
		}
		else{

			int checking_inode_num;		
			short result_block_num[12];
	
			read_inode(disk, saved_parent_inode_block_num, result_block_num);
			
			for(int i = 0; result_block_num[i] != -1; i++){
			
				search_file_or_dir(disk, result_block_num[i], curr_file_name, &checking_inode_num);
				
				if(checking_inode_num > 0){
					
					saved_parent_inode_block_num = findMapping(disk, checking_inode_num);
				}
			}
			
			if(checking_inode_num < 1){
				printf("\n      no such directory in this datablock: '%s'   \n", curr_file_name);
				break;
			}
		}
	}
	free(parent_directory_name);
	free(curr_file_name);
	free(fake_curr_dir_name);	
}

void list_file(FILE* disk, char* input){
	
	//printf("****************************%s \n", input);
	
	char* parent_directory_name = (char*)calloc(31,1);	
	char* curr_dir_name = (char*)calloc(31,1);	
	char* fake_curr_dir_name = (char*)calloc(31,1);	
	
	const char s[2] = "/";

	fake_curr_dir_name = strtok(input, s);		// skip the first command "Mkfile"

	strncpy(parent_directory_name, curr_dir_name, 31);
	strncpy(curr_dir_name, fake_curr_dir_name, 31);
	
	fake_curr_dir_name = strtok(NULL, s);

	int saved_parent_inode_block_num = -1;
	
		int newest_ROOT_INODE_INDEX = getRootInodeIndex(disk);
	
		int root_inode_block_num = findMapping(disk, newest_ROOT_INODE_INDEX);

		short result_block_num4[12];
		read_inode(disk, root_inode_block_num, result_block_num4);
	
		saved_parent_inode_block_num = root_inode_block_num;
	
		
		if(fake_curr_dir_name == NULL){

				short result_block_num[12];
				read_inode(disk, saved_parent_inode_block_num, result_block_num);	

				printf("\n\n         here is all the file in %s directory: \n\n", curr_dir_name);

				for(int i = 0;(result_block_num[i] != -1) &&(i<10); i++){

					short inode_num_empty = 0;
					char* buffer3 = (char*)calloc(BLOCK_SIZE, 1);
					readBlock(disk, result_block_num[i], buffer3, BLOCK_SIZE);

					for(int i = 0; i < 16; i++){

						char* buffer4 = (char*)calloc(31, 1);
						
						strncpy(buffer4, buffer3 +(i*32) + 1, 31);
						memcpy(&inode_num_empty, buffer3 +(i*32), 1);
							
						if(inode_num_empty != 0) {
							
							printf("         #### file name %d : %s\n", i, buffer4);
						}
						
						free(buffer4); 
					}
					free(buffer3);
				}
				printf("\n       $$$$$$$$$$$$$$ we listed all the file in %s directory \n\n", curr_dir_name);
		}


	while(fake_curr_dir_name != NULL) {

		strncpy(parent_directory_name, curr_dir_name, 31);
		strncpy(curr_dir_name, fake_curr_dir_name, 31);
		fake_curr_dir_name = strtok(NULL, s);
		//printf("curr_dir_name: %s ", curr_dir_name);
		//printf("parent_directory_name: %s\n", parent_directory_name);

		if(fake_curr_dir_name == NULL){
			
			int checking_inode_num;		
			short result_block_num3[12];
	
			read_inode(disk, saved_parent_inode_block_num, result_block_num3);
			
			for(int i = 0; result_block_num3[i] != -1; i++){
			
				search_file_or_dir(disk, result_block_num3[i], curr_dir_name, &checking_inode_num);
				
				if(checking_inode_num > 0){
					
					saved_parent_inode_block_num = findMapping(disk, checking_inode_num);
				}
			}
			if(checking_inode_num < 1){
				printf("\n      no such directory in this datablock: '%s'   \n", curr_dir_name);
				break;
			}

			//printf("*********************************************************************\n");

			short result_block_num[12];
			read_inode(disk, saved_parent_inode_block_num, result_block_num);	

			printf("\n\n         here is all the file in %s directory: \n\n", curr_dir_name);

			for(int i = 0;(result_block_num[i] != -1) &&(i<10); i++){

				short inode_num_empty = 0;
				char* buffer3 = (char*)calloc(BLOCK_SIZE, 1);
				readBlock(disk, result_block_num[i], buffer3, BLOCK_SIZE);

				for(int i = 0; i < 16; i++){

					char* buffer4 = (char*)calloc(31, 1);
					
					strncpy(buffer4, buffer3 +(i*32) + 1, 31);
					memcpy(&inode_num_empty, buffer3 +(i*32), 1);
						
					if(inode_num_empty != 0) {
						
						printf("         #### file name %d : %s\n", i, buffer4);
					}
					
					free(buffer4); 
				}
				free(buffer3);
			}
			printf("\n       $$$$$$$$$$$$$$ we listed all the file in %s directory \n\n", curr_dir_name);
		}
		else{

			int checking_inode_num;		
			short result_block_num[12];
	
			read_inode(disk, saved_parent_inode_block_num, result_block_num);
			
			for(int i = 0; result_block_num[i] != -1; i++){
			
				search_file_or_dir(disk, result_block_num[i], curr_dir_name, &checking_inode_num);
				
				if(checking_inode_num > 0){
					
					saved_parent_inode_block_num = findMapping(disk, checking_inode_num);
				}
			}
			
			if(checking_inode_num < 1){
				printf("\n      no such directory in this datablock: '%s'   \n", curr_dir_name);
				break;
			}
		}
	}
	free(parent_directory_name);
	free(curr_dir_name);
	free(fake_curr_dir_name);	
}



void Rm_dir(FILE* disk, char* input){
	
	char* parent_directory_name = (char*)calloc(31,1);	
	char* curr_dir_name = (char*)calloc(31,1);	
	char* fake_curr_dir_name = (char*)calloc(31,1);	
	
	const char s[2] = "/";
	
	fake_curr_dir_name = strtok(input, s);   // skip the first command "Mkfile"
	
	strncpy(parent_directory_name, curr_dir_name, 31);
	strncpy(curr_dir_name, fake_curr_dir_name, 31);
	
	fake_curr_dir_name = strtok(NULL, s);

	int saved_parent_inode_block_num = -1;
	
		int newest_ROOT_INODE_INDEX = getRootInodeIndex(disk);
			
		int root_inode_block_num = findMapping(disk, newest_ROOT_INODE_INDEX);

		short result_block_num4[12];
		read_inode(disk, root_inode_block_num, result_block_num4);
			
		saved_parent_inode_block_num = root_inode_block_num;
	
	while(fake_curr_dir_name != NULL) {

		strncpy(parent_directory_name, curr_dir_name, 31);
		strncpy(curr_dir_name, fake_curr_dir_name, 31);
		fake_curr_dir_name = strtok(NULL, s);
		//printf("curr_dir_name: %s ", curr_dir_name);
		//printf("parent_directory_name: %s\n", parent_directory_name);

		if(fake_curr_dir_name == NULL){
			
			short result_block_num[12];
	
			read_inode(disk, saved_parent_inode_block_num, result_block_num);

			int checking_inode_num = -1;		
			int file_appear_in_which_block_of_directory;
			
			for(int i = 0;(result_block_num[i] != -1) &&(checking_inode_num == -1); i++){
			
				search_file_or_dir(disk, result_block_num[i], curr_dir_name, &checking_inode_num);
				
				if(checking_inode_num != -1){
				
					file_appear_in_which_block_of_directory = i;
				}
			}
			if(checking_inode_num == -1){
				printf("\n      no such directory: '%s'   \n", curr_dir_name);
				break;
			}
			
			delete_directory(disk, result_block_num[file_appear_in_which_block_of_directory], curr_dir_name, checking_inode_num);

			printf("\n       $$$$$$$$$$$$$$ we deleted a directory, directory name: %s\n\n", curr_dir_name);
		}
		else{

			int checking_inode_num;		
			short result_block_num[12];
	
			read_inode(disk, saved_parent_inode_block_num, result_block_num);
			
			for(int i = 0; result_block_num[i] != -1; i++){
			
				search_file_or_dir(disk, result_block_num[i], curr_dir_name, &checking_inode_num);
				
				if(checking_inode_num > 0){
					
					saved_parent_inode_block_num = findMapping(disk, checking_inode_num);
				}
			}
			
			if(checking_inode_num < 1){
				printf("\n      no such directory in this datablock: '%s'   \n", curr_dir_name);
				break;
			}
		}
	}
	free(parent_directory_name);
	free(curr_dir_name);
	free(fake_curr_dir_name);	
}

void command_input(FILE* disk, char* input, char* file_content_larger){

	char tok_string7[10];
	strncpy(tok_string7, input, 6);
	char* command7 = (char*)calloc(60,1);	
	char s[2] = "/";
	command7 = strtok(tok_string7, s);		// skip the first command "open"
	
	char* input2 = (char*)calloc(60,1);
	strncpy(input2, input, 60);
	char* path = (char*)calloc(60,1);
	path = strtok(input2, "/");
	path = strtok(NULL, "");
	
	if(file_content_larger != NULL){
		robust_superblock(disk, input, strlen(file_content_larger));
	}else{
		robust_superblock(disk, input, 0);
	}
	
	if(path == NULL){
		path = "root";
	}
	
	printf("\n\ntest case %d: ", x++);

	if(strncmp(command7, "list", 4) == 0){		
	
		printf("we are listing the file in a directory at '%s'", path);
		list_file(disk, input);
	}
	else if(strncmp(command7, "Open", 4) == 0){
		
		printf("we are opening a file at '%s'", path);
		open_file(disk, input);
	}
	else if(strncmp(command7, "Rmdir", 5) == 0){		
		
		printf("we are deleting a directory at '%s'", path);
		Rm_dir(disk, input);
	}
	else if(strncmp(command7, "Rmfile", 6) == 0){
		
		printf("we are deleting a file at '%s'", path);
		Rm_file(disk, input);
	}
	else if((strncmp(command7, "Writefile", 6) == 0) && file_content_larger == NULL){

		printf("we are making a new empty file at '%s'", path);
		write_empty_file(disk, input);
	}
	else if(strncmp(command7, "Writefile", 6) == 0){			
		
		printf("we are making a new file with content at '%s'", path);
		write_file(disk, input, file_content_larger);
	}
	else if(strncmp(command7, "Mkdir", 5) == 0){		
		
		printf("we are making a directory at '%s'", path);
		make_directory(disk, input);
	}
}