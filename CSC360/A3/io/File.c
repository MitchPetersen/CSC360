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
	int freeInodeIndex = 0;
	for(int i = 9; (i < 24) && (signal == 0); i++){	
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
	return temp2;
}


void deleteMapping(FILE* disk, int inodeNumber){
	int temp = (inodeNumber-1) / 128; 
	int mappingBlock = MAPPING_BLOCK_START + temp;
	char* buffer = (char*)calloc(BLOCK_SIZE, 1);
	
	readBlock(disk, mappingBlock, buffer, BLOCK_SIZE);
	
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
		memcpy(singleIndirectContent + ((i-10)*2), &fileBlockArray[i], 2);
	}

	writeBlock(disk, freeBlockNum, singleIndirectContent, BLOCK_SIZE);
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


void createFileSingleInode(FILE* disk, int freeBlock, int directoryBlockNumber){
	char* inode = calloc(BLOCK_SIZE, 1);
	int fileSize = 0;
	int flags = 1;
	short dataBlock[12];
	
	for(int i = 0; i < 12; i++){
		dataBlock[i] = -1;
		memcpy(inode + 8 + (i*2), &dataBlock[i], 2);
	}

    short dataBlock1 = (short) directoryBlockNumber;
	
	memcpy(inode, &fileSize, 4);
	memcpy(inode+4, &flags, 4);
	memcpy(inode+8, &dataBlock1, 2);
	writeBlock(disk, freeBlock, inode, BLOCK_SIZE);
	free(inode);
}	
	
	
void createDirectoryInode(FILE* disk, int freeBlock, int directoryBlockNumber){
	char* inode = calloc(BLOCK_SIZE, 1);
	int fileSize = 0;
	int flags = 2;
	short dataBlock[12];
	
	for(int i = 0; i < 12; i++){
		dataBlock[i] = -1;
		memcpy(inode+8+(i*2), &dataBlock[i], 2);
	}

    short dataBlock1 = (short) directoryBlockNumber;
	
	memcpy(inode, &fileSize, 4);
	memcpy(inode+4, &flags, 4);
	memcpy(inode+8, &dataBlock1, 2);
	writeBlock(disk, freeBlock, inode, BLOCK_SIZE);
	free(inode);
}	


void readInode(FILE* disk, int inodeBlock, short* resultBlock){
	char* buffer = (char*)calloc(32, 1);
    readBlock(disk, inodeBlock, buffer, 32);

	for(int i = 0; i < 12; i++){
		memcpy(&resultBlock[i], buffer +(8 + 2*i), 2);
	}
}


void createDirectoryBlock(FILE* disk, int freeBlockNum){
	char* data = (char*)calloc(BLOCK_SIZE, 1);
	writeBlock(disk, freeBlockNum, data, BLOCK_SIZE);
	free(data);
}


void extendParentDirectoryBlock(FILE* disk, int parentDirectoryNode, int* newBlockNumber){
	int freeBlockNumber;
	
	readFreeBlockVector(disk, &freeBlockNumber);
	createDirectoryBlock(disk, freeBlockNumber);
	fillFreeBlockVector(disk, freeBlockNumber);
	memcpy(newBlockNumber, &freeBlockNumber, 4);

	int latestDataBlock;
	short resultBlock[12];
	
	readInode(disk, parentDirectoryNode, resultBlock);

	for(int i = 0; i < 10; i++){
		if(resultBlock[i] != -1){
			latestDataBlock = i;
		}
	}
	
	resultBlock[latestDataBlock+1] = (short)freeBlockNumber;
	
	char* inode = calloc(32, 1);
	
    readBlock(disk, parentDirectoryNode, inode, 32);				
	memcpy(inode+8+((latestDataBlock+1)*2), &resultBlock[latestDataBlock+1], 2);
	writeBlock(disk, parentDirectoryNode, inode, 32);
	free(inode);
}


void EditParentDirectoryBlock(FILE* disk, int parentDirectoryNode, int childDirectoryNode, char* childDirectoryName){
	int latestDataBlock;
	short resultBlock[12];
	readInode(disk, parentDirectoryNode, resultBlock);

	for(int i = 0; i < 10; i++){
		if(resultBlock[i] != -1){
			latestDataBlock = i;
		}
	}
	
	char* buffer = (char*)calloc(BLOCK_SIZE, 1);	
	readBlock(disk, resultBlock[latestDataBlock], buffer, BLOCK_SIZE);  
	int entryNumber = -1; 
	
	for(int i = 0;(i < 16)&&(entryNumber == -1); i++){
		unsigned char temp; 
		memcpy(&temp, buffer + (i*32), 1); 
		int temp2 = (int) temp; 
		
		if(temp2 == 0){
			entryNumber = i;
		}
	}
	
	if(entryNumber == -1){
		int new_block_num;
		extendParentDirectoryBlock(disk, parentDirectoryNode, &new_block_num);
		unsigned char inodeNumber = (char) childDirectoryNode;
		memcpy(buffer +(entryNumber*32), &inodeNumber, 1);
		strncpy(buffer +(entryNumber*32) +1, childDirectoryName, 31);
		writeBlock(disk, new_block_num, buffer, BLOCK_SIZE);
	}
	
	else{
		unsigned char inodeNumber = (char) childDirectoryNode;
		memcpy(buffer +(entryNumber*32), &inodeNumber, 1);
		strncpy(buffer +(entryNumber*32) +1, childDirectoryName, 31);
		writeBlock(disk, resultBlock[latestDataBlock], buffer, BLOCK_SIZE);
	}
	free(buffer);
}


void deleteEntryDirectoryBlock(FILE* disk, int parentDirectoryNode, int deleteFileInodeNumber){
	char* buffer = (char*)calloc(BLOCK_SIZE, 1);	
	readBlock(disk, parentDirectoryNode, buffer, BLOCK_SIZE);
	int entryNumber = -1;
	
	for(int i = 0;(i < 16)&&(entryNumber == -1); i++){
		unsigned char temp;
		memcpy(&temp, buffer +(i*32), 1);
		int temp2 = (int) temp;
		
		if(temp2 == deleteFileInodeNumber){
			entryNumber = i;
		}
	}
	
	char* temp3 = (char*)calloc(32, 1);	
	memcpy(buffer +(entryNumber*32), temp3, 32);
	writeBlock(disk, parentDirectoryNode, buffer, BLOCK_SIZE);
	free(buffer);
}



void searchFileOrDirectory(FILE* disk, int directoryBlockNumber, char* fileName, int* findingInodeNumber){
	int entryNumber = -1;
	char* buffer = (char*)calloc(BLOCK_SIZE, 1);
	readBlock(disk, directoryBlockNumber, buffer, BLOCK_SIZE);
	
	for(int i = 0;(i < 16)&&(entryNumber == -1); i++){
		char* buffer2 = (char*)calloc(31, 1);	
		strncpy(buffer2, buffer +(i*32) + 1, 30);
		if(strncmp(buffer2, fileName, 30) == 0){
			entryNumber = i;
		}
		free(buffer2); 
	}
	
	if(entryNumber != -1){
		unsigned char temp;
		memcpy(&temp, buffer +(entryNumber*32), 1);
		int temp2 = (int) temp;
		memcpy(findingInodeNumber, &temp2, 4);
	} else {
		int temp2 = -1;
		memcpy(findingInodeNumber, &temp2, 4);
	}
	free(buffer);
}


void createFileBlock(FILE* disk, int freeBlock, char* fileContent){
	writeBlock(disk, freeBlock, fileContent, BLOCK_SIZE);
}


void createRoot(FILE* disk){
	int freeBlockNum;
	readFreeBlockVector(disk, &freeBlockNum);
	fillFreeBlockVector(disk, freeBlockNum);
	int rootDirectoryBlockNumber = freeBlockNum;     
	int freeBlock;
	readFreeBlockVector(disk, &freeBlock);
	createDirectoryInode(disk, freeBlock, rootDirectoryBlockNumber);
	fillFreeBlockVector(disk, freeBlock);
	int nextFree;
	findNextFreeInodenum(disk, &nextFree);
	addMapping(disk, nextFree, freeBlock);
	ROOT_INODE_INDEX = nextFree;
	updateSuperblock(disk);
}


void initLLFS(FILE* disk){
	createSuperblock(disk);
	createFreeBlockVector(disk);
	createRoot(disk);
	NUM_INODE++;
	updateSuperblock(disk);
}


void create_sub_directory(FILE* disk, int parentDirectoryNode, char* childDirectoryName){
	int freeBlock;
	readFreeBlockVector(disk, &freeBlock);
	createDirectoryBlock(disk, freeBlock);
	fillFreeBlockVector(disk, freeBlock);
	int subDirectryBlockNumber = freeBlock;    
	int freeBlock2; 
	readFreeBlockVector(disk, &freeBlock2);
	createDirectoryInode(disk, freeBlock2, subDirectryBlockNumber);
	fillFreeBlockVector(disk, freeBlock2);
	int nextFreeInode;
	findNextFreeInodenum(disk, &nextFreeInode);
	addMapping(disk, nextFreeInode, freeBlock2);
	NUM_INODE++;
	updateSuperblock(disk);
	int childDirectoryInode = nextFreeInode;   
	EditParentDirectoryBlock(disk, parentDirectoryNode, childDirectoryInode, childDirectoryName);
}


void createFile(FILE* disk, char* fileContent, int parentDirectoryInode, char* fileName){
	if(strlen(fileContent) < 512){
		int freeBlock;
		char* fileContentBuffer = (char*)calloc(BLOCK_SIZE, 1);
		
		strncpy(fileContentBuffer, fileContent, strlen(fileContent));
		readFreeBlockVector(disk, &freeBlock);
		createFileBlock(disk, freeBlock, fileContentBuffer);
		fillFreeBlockVector(disk, freeBlock);
		
		int fileBlockNum = freeBlock;   
		int freeBlock2;

		readFreeBlockVector(disk, &freeBlock2);
		createFileSingleInode(disk, freeBlock2, fileBlockNum);
		fillFreeBlockVector(disk, freeBlock2);

		int nextFree;
		
		findNextFreeInodenum(disk, &nextFree);
		addMapping(disk, nextFree, freeBlock2);
		
		NUM_INODE++;
		updateSuperblock(disk);

		int fileInode = nextFree;   
		EditParentDirectoryBlock(disk, parentDirectoryInode, fileInode, fileName);
	}
	else {
		
		int blockNeeded;
		
		if((strlen(fileContent) % 512) != 0){
			blockNeeded = (strlen(fileContent))/512+1;
		}else{
			blockNeeded = (strlen(fileContent))/512;
		}
			
		short storeBlockArray[blockNeeded];
			
		for(int i = 0; i < blockNeeded; i++){

			int freeBlock;
			
			char* fileContentBuffer = (char*)calloc(BLOCK_SIZE, 1);
			strncpy(fileContentBuffer, fileContent +(i*512), 512);

			readFreeBlockVector(disk, &freeBlock);
			createFileBlock(disk, freeBlock, fileContentBuffer);
			fillFreeBlockVector(disk, freeBlock);

			storeBlockArray[i] = (short) freeBlock;

		}

		int arraySize = blockNeeded;
		
		int freeBlock2;
		int fileSize = strlen(fileContent);

		readFreeBlockVector(disk, &freeBlock2);
		fillFreeBlockVector(disk, freeBlock2);
		createFileInode(disk, freeBlock2, storeBlockArray, arraySize, fileSize);

		int nextFree;
		
		findNextFreeInodenum(disk, &nextFree);
		addMapping(disk, nextFree, freeBlock2);

		NUM_INODE++;
		updateSuperblock(disk);

		int fileInodeNum = nextFree;   
		EditParentDirectoryBlock(disk, parentDirectoryInode, fileInodeNum, fileName);
	}
}


void createEmptyFile(FILE* disk, int parentDirectoryInode, char* fileName){
		int freeBlock;
		char* file_content_buffer = (char*)calloc(BLOCK_SIZE, 1);
		
		readFreeBlockVector(disk, &freeBlock);
		createFileBlock(disk, freeBlock, file_content_buffer);
		fillFreeBlockVector(disk, freeBlock);

		int fileBlock = freeBlock;   
		int freeBlock2;

		readFreeBlockVector(disk, &freeBlock2);
		createDirectoryInode(disk, freeBlock2, fileBlock);
		fillFreeBlockVector(disk, freeBlock2);
		
		int nextFree;
		
		findNextFreeInodenum(disk, &nextFree);
		addMapping(disk, nextFree, freeBlock2);
		NUM_INODE++;
		updateSuperblock(disk);

		int fileInode = nextFree;   
		EditParentDirectoryBlock(disk, parentDirectoryInode, fileInode, fileName);
}


void openFile(FILE* disk, char* input){

	char* parentDirectoryName = (char*)calloc(31,1);	
	char* currentFileName = (char*)calloc(31,1);	
	char* fakeCurrentFileName = (char*)calloc(31,1);	
	const char s[2] = "/";

	fakeCurrentFileName = strtok(input, s);
	strncpy(parentDirectoryName, currentFileName, 31);
	strncpy(currentFileName, fakeCurrentFileName, 31);
	fakeCurrentFileName = strtok(NULL, s);
	
	int savedParentInode = -1;
	int newestROOTINODEINDEX = getRootInodeIndex(disk);
	int rootInodeBlockNum = findMapping(disk, newestROOTINODEINDEX);
	short resultBlock[12];
	
	readInode(disk, rootInodeBlockNum, resultBlock);
	savedParentInode = rootInodeBlockNum;	
	
	while(fakeCurrentFileName != NULL) {
		strncpy(parentDirectoryName, currentFileName, 31);
		strncpy(currentFileName, fakeCurrentFileName, 31);
		fakeCurrentFileName = strtok(NULL, s);
		
		if(fakeCurrentFileName == NULL){
			int checkingInode;		
			short resultBlock2[12];
			readInode(disk, savedParentInode, resultBlock2);
			
			for(int i = 0; resultBlock2[i] != -1; i++){
				searchFileOrDirectory(disk, resultBlock2[i], currentFileName, &checkingInode);
				
				if(checkingInode > 0){
					savedParentInode = findMapping(disk, checkingInode);
				}
			}

			if(checkingInode < 1){
				printf("\n	# No such directory or file in this datablock: '%s'\n", currentFileName);
				break;
			}
			
			short resultBlockNum[12];
			readInode(disk, savedParentInode, resultBlockNum);
			
			if(resultBlockNum[10] > 0){

				short result_size;
				IndirectToSize(disk, resultBlockNum[10], &result_size);
				short result_block_num5[result_size];
				readInode(disk, savedParentInode, result_block_num5);	
				IndirectToArray(disk, resultBlockNum[10], result_block_num5, result_size);
				printf("\n	# Content of the file: \n");
				
				for(int i = 0;(result_block_num5[i] != -1) &&(i<result_size); i++){
					char* buffer = (char*)calloc(BLOCK_SIZE,1);	
					readBlock(disk,(int)result_block_num5[i], buffer, BLOCK_SIZE);
					printf("%s", buffer);
					free(buffer);
				}
				
				printf("\n\n");
			} else {
				printf("\n	# Content of the file: \n");

				for(int i = 0;(resultBlockNum[i] != -1) &&(i<10); i++){
					char* buffer = (char*)calloc(BLOCK_SIZE,1);	
					readBlock(disk,(int)resultBlockNum[i], buffer, BLOCK_SIZE);
					printf("%s", buffer);
					free(buffer);
				}
				
				printf("\n\n");
			}
		}

		else {
		
			int checkingInode;		
			short resultBlockNum[12];
			readInode(disk, savedParentInode, resultBlockNum);
			
			for(int i = 0; resultBlockNum[i] != -1; i++){
				searchFileOrDirectory(disk, resultBlockNum[i], currentFileName, &checkingInode);

				if(checkingInode > 0){
					savedParentInode = findMapping(disk, checkingInode);
				}
			}
			
			if(checkingInode < 1){
				printf("\n	# No such directory in this datablock: '%s'\n", currentFileName);
				break;
			}
		}
	}
	
	free(parentDirectoryName);
	free(currentFileName);
	free(fakeCurrentFileName);	
}


void makeDirectory(FILE* disk, char* input){
	char* parentDirectoryName = (char*)calloc(31,1);	
	char* currentDirectoryName = (char*)calloc(31,1);	
	char* fakeCurrentDirectoryName = (char*)calloc(31,1);	
	const char s[2] = "/";
	
	fakeCurrentDirectoryName = strtok(input, s);		// skip "Mkdir"
	strncpy(parentDirectoryName, currentDirectoryName, 31);
	strncpy(currentDirectoryName, fakeCurrentDirectoryName, 31);
	fakeCurrentDirectoryName = strtok(NULL, s);
	
	int savedParentInodeBlock = -1;
	int newestROOTINODEINDEX = getRootInodeIndex(disk);
	int rootInodeBlockNum = findMapping(disk, newestROOTINODEINDEX);
	short resultBlock[12];
	
	readInode(disk, rootInodeBlockNum, resultBlock);
	savedParentInodeBlock = rootInodeBlockNum;
	
	while(fakeCurrentDirectoryName != NULL) {
		strncpy(parentDirectoryName, currentDirectoryName, 31);
		strncpy(currentDirectoryName, fakeCurrentDirectoryName, 31);
		fakeCurrentDirectoryName = strtok(NULL, s);
		printf("currentDirectoryName: %s ", currentDirectoryName);
		printf("parentDirectoryName: %s\n", parentDirectoryName);

		if(fakeCurrentDirectoryName == NULL){
			create_sub_directory(disk, savedParentInodeBlock, currentDirectoryName);
			printf("\n	# We added a sub directory: %s\n", currentDirectoryName);
		} else {
			int checking_inode_num;		
			short result_block_num[12];
			readInode(disk, savedParentInodeBlock, result_block_num);
			
			for(int i = 0; result_block_num[i] != -1; i++){
				searchFileOrDirectory(disk, result_block_num[i], currentDirectoryName, &checking_inode_num);
				
				if(checking_inode_num > 0){
					savedParentInodeBlock = findMapping(disk, checking_inode_num);
				}
			}
			
			if(checking_inode_num < 1){
				printf("\n	# No such directory in this datablock: '%s'\n", currentDirectoryName);
				break;
			}
		}
	}
	free(parentDirectoryName);
	free(currentDirectoryName);
	free(fakeCurrentDirectoryName);	
}


void writeFile(FILE* disk, char* input, char* fileContent){
	char* parentDirectoryName = (char*)calloc(31,1);	
	char* currentFileName = (char*)calloc(31,1);	
	char* fakeCurrentDirectoryName = (char*)calloc(31,1);	
	const char s[2] = "/";

	fakeCurrentDirectoryName = strtok(input, s);		// skip the first command
	strncpy(parentDirectoryName, currentFileName, 31);
	strncpy(currentFileName, fakeCurrentDirectoryName, 31);
	fakeCurrentDirectoryName = strtok(NULL, s);
	
	int savedParent = -1;
	int newestROOTINODEINDEX = getRootInodeIndex(disk);
	int rootInodeBlockNum = findMapping(disk, newestROOTINODEINDEX);
	short resultBlock[12];
	
	readInode(disk, rootInodeBlockNum, resultBlock);
	savedParent = rootInodeBlockNum;
	
	while(fakeCurrentDirectoryName != NULL) {
		strncpy(parentDirectoryName, currentFileName, 31);
		strncpy(currentFileName, fakeCurrentDirectoryName, 31);
		fakeCurrentDirectoryName = strtok(NULL, s);

		if(fakeCurrentDirectoryName == NULL){
			createFile(disk, fileContent, savedParent, currentFileName);
			printf("\n	# We added a file, file name: %s\n", currentFileName);
		} else {
			int checking_inode_num;		
			short resultBlock2[12];
			readInode(disk, savedParent, resultBlock2);
			
			for(int i = 0; resultBlock2[i] != -1; i++){
				searchFileOrDirectory(disk, resultBlock2[i], currentFileName, &checking_inode_num);
				
				if(checking_inode_num > 0){
					savedParent = findMapping(disk, checking_inode_num);
				}
			}
			
			if(checking_inode_num < 1){
				printf("\n	# No such directory in this datablock: '%s'\n", currentFileName);
				break;
			}
		}
	}
	free(parentDirectoryName);
	free(currentFileName);
	free(fakeCurrentDirectoryName);	
}


void writeEmptyFile(FILE* disk, char* input){
	
	char* parentDirectoryName = (char*)calloc(31,1);	
	char* currentFileName = (char*)calloc(31,1);	
	char* fakeCurrentDirectoryName = (char*)calloc(31,1);	
	const char s[2] = "/";

	fakeCurrentDirectoryName = strtok(input, s);		// skip the first command
	strncpy(parentDirectoryName, currentFileName, 31);
	strncpy(currentFileName, fakeCurrentDirectoryName, 31);
	fakeCurrentDirectoryName = strtok(NULL, s);
	
	int savedParentInode = -1;
	int newestROOTINODEINDEX = getRootInodeIndex(disk);
	int rootInodeBlockNum = findMapping(disk, newestROOTINODEINDEX);
	short resultBlock[12];
	
	readInode(disk, rootInodeBlockNum, resultBlock);
	savedParentInode = rootInodeBlockNum;
	
	while(fakeCurrentDirectoryName != NULL) {
		strncpy(parentDirectoryName, currentFileName, 31);
		strncpy(currentFileName, fakeCurrentDirectoryName, 31);
		fakeCurrentDirectoryName = strtok(NULL, s);
		
		if(fakeCurrentDirectoryName == NULL){
			createEmptyFile(disk, savedParentInode, currentFileName);
			printf("\n	# We added an empty file, file name: %s\n\n", currentFileName);
		} else {
			int checkingInode;		
			short resultBlock2[12];
			readInode(disk, savedParentInode, resultBlock2);
			
			for(int i = 0; resultBlock2[i] != -1; i++){
				searchFileOrDirectory(disk, resultBlock2[i], currentFileName, &checkingInode);
				
				if(checkingInode > 0){
					savedParentInode = findMapping(disk, checkingInode);
				}
			}
			
			if(checkingInode < 1){
				printf("\n	# No such directory in this datablock: '%s'\n", currentFileName);
				break;
			}
		}
	}
	free(parentDirectoryName);
	free(currentFileName);
	free(fakeCurrentDirectoryName);	
}


void delete_file(FILE* disk, int parentDirectoryBlockNumber, char* currentFileName, int deleteFileInodeNumber){
	short deleteBlock[12];
	int deleteInodeBlockNumber = findMapping(disk, deleteFileInodeNumber);
	readInode(disk, deleteInodeBlockNumber, deleteBlock);

	for(int i = 0; deleteBlock[i] != -1; i++){
		deleteFreeBlockVector(disk, deleteBlock[i]);
	}
	
	deleteFreeBlockVector(disk, deleteInodeBlockNumber);
	NUM_INODE--;
	updateSuperblock(disk);
	deleteMapping(disk, deleteFileInodeNumber);
	deleteEntryDirectoryBlock(disk, parentDirectoryBlockNumber, deleteFileInodeNumber);
}


void deleteDirectory(FILE* disk, int parentDirectoryBlockNumber, char* currentFileName, int deleteDirectoryInodeNumber){
	short delBlockNumber[12];
	int deleteInodeBlockNumber = findMapping(disk, deleteDirectoryInodeNumber);
	readInode(disk, deleteInodeBlockNumber, delBlockNumber);
	
	for(int i = 0; delBlockNumber[i] != -1; i++){
		deleteFreeBlockVector(disk, delBlockNumber[i]);
	}
	
	deleteFreeBlockVector(disk, deleteDirectoryInodeNumber);
	NUM_INODE--;
	updateSuperblock(disk);
	deleteMapping(disk, deleteDirectoryInodeNumber);
	deleteEntryDirectoryBlock(disk, parentDirectoryBlockNumber, deleteDirectoryInodeNumber);
}


void rmFile(FILE* disk, char* input){
	char* parentDirectoryName = (char*)calloc(31,1);	
	char* currentFileName = (char*)calloc(31,1);	
	char* fakeCurrentDirectoryName = (char*)calloc(31,1);	
	const char s[2] = "/";

	fakeCurrentDirectoryName = strtok(input, s);		// skip the first command
	strncpy(parentDirectoryName, currentFileName, 31);
	strncpy(currentFileName, fakeCurrentDirectoryName, 31);
	fakeCurrentDirectoryName = strtok(NULL, s);
	
	int savedParentInodeBlockNumber = -1;
	int newestROOTINODEINDEX = getRootInodeIndex(disk);
	int rootInodeBlockNumber = findMapping(disk, newestROOTINODEINDEX);
	short resultBlock4[12];
	
	readInode(disk, rootInodeBlockNumber, resultBlock4);
	savedParentInodeBlockNumber = rootInodeBlockNumber;
	
	while(fakeCurrentDirectoryName != NULL) {
		strncpy(parentDirectoryName, currentFileName, 31);
		strncpy(currentFileName, fakeCurrentDirectoryName, 31);
		fakeCurrentDirectoryName = strtok(NULL, s);

		if(fakeCurrentDirectoryName == NULL){
			short resultBlock[12];
			readInode(disk, savedParentInodeBlockNumber, resultBlock);
			int checkingInodeNum = -1;		
			int fileAppearInWhichBlockOfDirectory;
			
			for(int i = 0;(resultBlock[i] != -1) &&(checkingInodeNum == -1); i++){
				searchFileOrDirectory(disk, resultBlock[i], currentFileName, &checkingInodeNum);
				
				if(checkingInodeNum != -1){
					fileAppearInWhichBlockOfDirectory = i;
				}
			}
			
			if(checkingInodeNum == -1){
				printf("	# File not exist\n");
				break;
			}
			
			delete_file(disk, resultBlock[fileAppearInWhichBlockOfDirectory], currentFileName, checkingInodeNum);
			printf("\n	# We deleted a file, file name: %s\n", currentFileName);
		} else {
			int checkingInodeNum;		
			short resultBlock[12];
			readInode(disk, savedParentInodeBlockNumber, resultBlock);
			
			for(int i = 0; resultBlock[i] != -1; i++){
				searchFileOrDirectory(disk, resultBlock[i], currentFileName, &checkingInodeNum);
				
				if(checkingInodeNum > 0){
					savedParentInodeBlockNumber = findMapping(disk, checkingInodeNum);
				}
			}
			
			if(checkingInodeNum < 1){
				printf("\n	# No such directory in this datablock: '%s'\n", currentFileName);
				break;
			}
		}
	}
	free(parentDirectoryName);
	free(currentFileName);
	free(fakeCurrentDirectoryName);	
}

void listFile(FILE* disk, char* input, char* path){	
	char* parentDirectoryName = (char*)calloc(31,1);	
	char* currentDirectoryName = (char*)calloc(31,1);	
	char* fakeCurrentDirectoryName = (char*)calloc(31,1);	
	const char s[2] = "/";

	fakeCurrentDirectoryName = strtok(input, s);		// skip the first command
	strncpy(parentDirectoryName, currentDirectoryName, 31);
	strncpy(currentDirectoryName, fakeCurrentDirectoryName, 31);
	fakeCurrentDirectoryName = strtok(NULL, s);
	
	int savedParentInodeBlockNumber = -1;
	int newestROOTINODEINDEX = getRootInodeIndex(disk);
	int root_inode_block_num = findMapping(disk, newestROOTINODEINDEX);
	short result_block_num4[12];
	
	readInode(disk, root_inode_block_num, result_block_num4);
	savedParentInodeBlockNumber = root_inode_block_num;
	
	if(fakeCurrentDirectoryName == NULL){
			short result_block_num[12];
			readInode(disk, savedParentInodeBlockNumber, result_block_num);	
			printf("\n	# Here is all the file in %s directory: \n", path);

			for(int i = 0;(result_block_num[i] != -1) &&(i<10); i++){
				short inode_num_empty = 0;
				char* buffer3 = (char*)calloc(BLOCK_SIZE, 1);
				readBlock(disk, result_block_num[i], buffer3, BLOCK_SIZE);

				for(int i = 0; i < 16; i++){
					char* buffer4 = (char*)calloc(31, 1);
					strncpy(buffer4, buffer3 +(i*32) + 1, 31);
					memcpy(&inode_num_empty, buffer3 +(i*32), 1);
						
					if(inode_num_empty != 0) {
						printf("	# File name %d : %s\n", i, buffer4);
					}
					free(buffer4); 
				}
				free(buffer3);
			}
			printf("\n	# We listed all the files in %s directory \n", path);
	}

	while(fakeCurrentDirectoryName != NULL) {
		strncpy(parentDirectoryName, currentDirectoryName, 31);
		strncpy(currentDirectoryName, fakeCurrentDirectoryName, 31);
		fakeCurrentDirectoryName = strtok(NULL, s);
		
		if(fakeCurrentDirectoryName == NULL){
			int checking_inode_num;		
			short result_block_num3[12];
			readInode(disk, savedParentInodeBlockNumber, result_block_num3);
			
			for(int i = 0; result_block_num3[i] != -1; i++){
				searchFileOrDirectory(disk, result_block_num3[i], currentDirectoryName, &checking_inode_num);
				
				if(checking_inode_num > 0){
					savedParentInodeBlockNumber = findMapping(disk, checking_inode_num);
				}
			}
			
			if(checking_inode_num < 1){
				printf("\n # No such directory in this datablock: '%s'   \n", path);
				break;
			}

			short result_block_num[12];
			readInode(disk, savedParentInodeBlockNumber, result_block_num);	
			printf("\n	# Here is all the file in %s directory: \n", path);

			for(int i = 0;(result_block_num[i] != -1) &&(i<10); i++){
				short inode_num_empty = 0;
				char* buffer3 = (char*)calloc(BLOCK_SIZE, 1);
				readBlock(disk, result_block_num[i], buffer3, BLOCK_SIZE);

				for(int i = 0; i < 16; i++){
					char* buffer4 = (char*)calloc(31, 1);
					strncpy(buffer4, buffer3 +(i*32) + 1, 31);
					memcpy(&inode_num_empty, buffer3 +(i*32), 1);

					if(inode_num_empty != 0) {
						printf("	# file name %d : %s\n", i, buffer4);
					}
					free(buffer4); 
				}
				free(buffer3);
			}
			printf("\n	# We listed all the file in %s directory\n", path);
		} else {
			int checking_inode_num;		
			short result_block_num[12];
			readInode(disk, savedParentInodeBlockNumber, result_block_num);
		
			for(int i = 0; result_block_num[i] != -1; i++){
				searchFileOrDirectory(disk, result_block_num[i], currentDirectoryName, &checking_inode_num);

				if(checking_inode_num > 0){
					savedParentInodeBlockNumber = findMapping(disk, checking_inode_num);
				}
			}
			
			if(checking_inode_num < 1){
				printf("\n # No such directory in this datablock: '%s'   \n", path);
				break;
			}
		}
	}
	free(parentDirectoryName);
	free(currentDirectoryName);
	free(fakeCurrentDirectoryName);	
}



void rmDir(FILE* disk, char* input){
	
	char* parentDirectoryName = (char*)calloc(31,1);	
	char* currentDirectoryName = (char*)calloc(31,1);	
	char* fakeCurrentDirectoryName = (char*)calloc(31,1);	
	const char s[2] = "/";
	
	fakeCurrentDirectoryName = strtok(input, s);   // skip the first command "Mkfile"
	strncpy(parentDirectoryName, currentDirectoryName, 31);
	strncpy(currentDirectoryName, fakeCurrentDirectoryName, 31);
	fakeCurrentDirectoryName = strtok(NULL, s);

	int savedParentInodeBlockNumber = -1;
	int newestROOTINODEINDEX = getRootInodeIndex(disk);
	int rootInodeBlockNumber = findMapping(disk, newestROOTINODEINDEX);
	short resultBlock4[12];
	
	readInode(disk, rootInodeBlockNumber, resultBlock4);
	savedParentInodeBlockNumber = rootInodeBlockNumber;

	while(fakeCurrentDirectoryName != NULL) {
		strncpy(parentDirectoryName, currentDirectoryName, 31);
		strncpy(currentDirectoryName, fakeCurrentDirectoryName, 31);
		fakeCurrentDirectoryName = strtok(NULL, s);

		if(fakeCurrentDirectoryName == NULL){
			short resultBlock[12];
			readInode(disk, savedParentInodeBlockNumber, resultBlock);
			int checkingInodeNumber = -1;		
			int file_appear_in_which_block_of_directory;
			
			for(int i = 0;(resultBlock[i] != -1) &&(checkingInodeNumber == -1); i++){
				searchFileOrDirectory(disk, resultBlock[i], currentDirectoryName, &checkingInodeNumber);
				
				if(checkingInodeNumber != -1){
					file_appear_in_which_block_of_directory = i;
				}
			}
			if(checkingInodeNumber == -1){
				printf("\n	# No such directory: '%s'\n", currentDirectoryName);
				break;
			}
			
			deleteDirectory(disk, resultBlock[file_appear_in_which_block_of_directory], currentDirectoryName, checkingInodeNumber);
			printf("\n	# We deleted a directory, directory name: %s\n", currentDirectoryName);
		} else {
			int checkingInodeNumber;		
			short resultBlock[12];
			readInode(disk, savedParentInodeBlockNumber, resultBlock);
			
			for(int i = 0; resultBlock[i] != -1; i++){
				searchFileOrDirectory(disk, resultBlock[i], currentDirectoryName, &checkingInodeNumber);
				
				if(checkingInodeNumber > 0){
					savedParentInodeBlockNumber = findMapping(disk, checkingInodeNumber);
				}
			}
			if(checkingInodeNumber < 1){
				printf("\n	# No such directory in this datablock: '%s'\n", currentDirectoryName);
				break;
			}
		}
	}
	free(parentDirectoryName);
	free(currentDirectoryName);
	free(fakeCurrentDirectoryName);	
}

void command_input(FILE* disk, char* input, char* fileContent){

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
	
	if(fileContent != NULL){
		robustSuperblock(disk, input, strlen(fileContent));
	}else{
		robustSuperblock(disk, input, 0);
	}
	
	if(path == NULL){
		path = "root";
	}
	
	printf("\n\nTest Case %d: ", x++);
	if(strncmp(command7, "list", 4) == 0){		
		printf("Listing the file in a directory at '%s'", path);
		listFile(disk, input, path);
	} else if(strncmp(command7, "Open", 4) == 0){
		printf("Opening a file at '%s'", path);
		openFile(disk, input);
	} else if(strncmp(command7, "Rmdir", 5) == 0){		
		printf("Deleting a directory at '%s'", path);
		rmDir(disk, input);
	} else if(strncmp(command7, "Rmfile", 6) == 0){
		printf("Deleting a file at '%s'", path);
		rmFile(disk, input);
	} else if((strncmp(command7, "Writefile", 6) == 0) && fileContent == NULL){
		printf("Making a new empty file at '%s'", path);
		writeEmptyFile(disk, input);
	} else if(strncmp(command7, "Writefile", 6) == 0){			
		printf("Making a new file with content at '%s'", path);
		writeFile(disk, input, fileContent);
	} else if(strncmp(command7, "Mkdir", 5) == 0){		
		printf("Making a directory at '%s'", path);
		makeDirectory(disk, input);
	}
}