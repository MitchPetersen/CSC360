#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

const int blockSize = 512;
const int numberBlocks = 4096;
const int InodeSize = 32;

int inodeNum = 0;
int rootInodeIndex = -1;
int magic = 1;
int blockMappingStart = 9; // block 9-24 for inode #1 - #2048
int x = 1;


void writeBlock(FILE* disk, int blockNumber, char* data, int size){
    fseek(disk, blockNumber * blockSize, SEEK_SET);
    fwrite(data, size, 1, disk); 
}


void readBlock(FILE* disk, int blockNumber, char* buffer, int size){
    fseek(disk, blockNumber * blockSize, SEEK_SET);
    fread(buffer, size, 1, disk);
}


void createSuperblock(FILE* disk){
	char* buffer = (char*)calloc(blockSize, 1);
	memcpy(buffer, &magic, 4);
	memcpy(buffer+4, &numberBlocks, 4);
	memcpy(buffer+8, &inodeNum, 4);
	memcpy(buffer+12, &rootInodeIndex, 4);
	memcpy(buffer+20, "NONE", 60);
	writeBlock(disk, 0, buffer, blockSize);
	free(buffer);
}


void readSuperblock(FILE* disk){ // reads block 0
	char* buffer = (char*)calloc(blockSize, 1);
    readBlock(disk, 0, buffer, blockSize);	
	memcpy(&inodeNum, buffer+8, 4);
	memcpy(&rootInodeIndex, buffer+12, 4);
	free(buffer);
}


int getRootInodeIndex(FILE* disk){
	
	char* buffer = (char*)calloc(blockSize, 1);
    readBlock(disk, 0, buffer, blockSize);	
	
	int tempRootInodeIndex;
	memcpy(&tempRootInodeIndex, buffer+12, 4);
	
	free(buffer);
	
	return tempRootInodeIndex;
}


void updateSuperblock(FILE* disk){
	char* buffer = (char*)calloc(blockSize, 1);
    readBlock(disk, 0, buffer, blockSize);
	memcpy(buffer+4, &numberBlocks, 4);
	memcpy(buffer+8, &inodeNum, 4);
	memcpy(buffer+12, &rootInodeIndex, 4);
	writeBlock(disk, 0, buffer, blockSize);
	free(buffer);
}


void robustSuperblock(FILE* disk, char* input, int contentLength){
	char* buffer = (char*)calloc(blockSize, 1);
    readBlock(disk, 0, buffer, blockSize);
	
	memcpy( buffer+16, &contentLength, 4);
	memcpy( buffer+20, input, 60);
	
	writeBlock(disk, 0, buffer, blockSize);	
	free(buffer);
}


void createFreeBlockVector(FILE* disk){
	// block 1 - 8
	for(int j = 1; j < 9; j++) {
		if(j == 1){
			char freeBlockContent[blockSize];
			for(int i = 0; i < blockSize; i++){
				if(i < 25){
					freeBlockContent[i] = '0';
				} else {
					freeBlockContent[i] = '1';
				}
			}
			writeBlock(disk, 1, freeBlockContent, blockSize);
		} else {
			char freeBlockContent2[blockSize];
			for(int i = 0; i < blockSize; i++){
				freeBlockContent2[i] = '1';
			}

			writeBlock(disk, j, freeBlockContent2, blockSize);
		}
	}
}


void readFreeBlockVector(FILE* disk, int* findFreeBlockNum){      
	// block 1 - 8
	int findSignal = 0;
	int freeBlockIndex = 0;
	
	for(int i = 1;(i < 9)&&(findSignal == 0); i++){	
		char* buffer5 = (char*)calloc(blockSize, 1);
		readBlock(disk, i, buffer5, blockSize);
		
		for(int j=0;(j < blockSize)&&(findSignal == 0); j++){
			if(buffer5[j] == '1'){
				findSignal = 1;
				freeBlockIndex = j +((i-1)*512);
			}			
		}
		free(buffer5);	
	}
	memcpy(findFreeBlockNum, &freeBlockIndex, 4);
}


void fillFreeBlockVector(FILE* disk, int fill_block_num){ 
    // block 1 - 8
	int freeBlockNumber = fill_block_num / 128; 
	int remainder = fill_block_num % 128;
	char* buffer5 = (char*)calloc(blockSize, 1);	

	readBlock(disk, freeBlockNumber+1, buffer5, blockSize);
	buffer5[remainder] = '0';
	writeBlock(disk, freeBlockNumber+1, buffer5, blockSize);
	
	free(buffer5);		
}


void deleteFreeBlockVector(FILE* disk, int blockNumberToDelete){  
    // block 1 - 8
	int freeBlockNumber = blockNumberToDelete / 128; 
	int remainder = blockNumberToDelete % 128;
	char* buffer5 = (char*)calloc(blockSize, 1);	

	readBlock(disk, freeBlockNumber+1, buffer5, blockSize);
	buffer5[remainder] = '1';
	writeBlock(disk, freeBlockNumber+1, buffer5, blockSize);
	
	free(buffer5);	
}


void addMapping(FILE* disk, int inodeNum, int inodeBlockNumber){
	int temp = (inodeNum-1) / 128;
	int mappingBlock = blockMappingStart + temp;
	char* buffer = (char*)calloc(blockSize, 1);

	readBlock(disk, mappingBlock, buffer, blockSize);
	memcpy(buffer +((inodeNum-1) * 4), &inodeBlockNumber, 4); 
	writeBlock(disk, mappingBlock, buffer, blockSize);
	free(buffer);	
}


void findNextFreeInode(FILE* disk, int* inodeNum){
	int findSignal = 0;
	int freeInodeIndex = 0;

	for(int i = 9;(i < 24)&&(findSignal == 0); i++){	
		char* buffer = (char*)calloc(blockSize, 1);
		readBlock(disk, i, buffer, blockSize);
		for(int j=0;(j < 128)&&(findSignal == 0); j++){
			int tempInodeBlockNum;
			memcpy(&tempInodeBlockNum, (buffer +(j*4)), 4); 
			if(tempInodeBlockNum == 0){
				findSignal = 1;
				freeInodeIndex = j +((i-9)*128) + 1;
			}			
		}
		free(buffer);	
	}
	memcpy(inodeNum, &freeInodeIndex, 4);
}


int findMapping(FILE* disk, int inodeNum){

	int temp = (inodeNum-1) / 128; 
	int mappingBlock = blockMappingStart + temp;
	char* buffer = (char*)calloc(blockSize, 1);
	readBlock(disk, mappingBlock, buffer, blockSize);
	
	int tempBlockNum;
	memcpy(&tempBlockNum, buffer +((inodeNum-1) * 4), 4); 

	free(buffer);	
	return tempBlockNum;
}


void deleteMapping(FILE* disk, int inodeNum){

	int temp = (inodeNum-1) / 128; 
	int mappingBlock = blockMappingStart + temp;

	char* buffer = (char*)calloc(blockSize, 1);
	readBlock(disk, mappingBlock, buffer, blockSize);
	
	int clearInodeBlockNum = 0;
	memcpy(buffer +((inodeNum-1) * 4), &clearInodeBlockNum, 4); 
	writeBlock(disk, mappingBlock, buffer, blockSize);
	free(buffer);	
}


void create_singleIndirect(FILE* disk, short* blockNumberArray, int arraySize, short* indirectBlockNumber){
	int freeBlockNumber;
	readFreeBlockVector(disk, &freeBlockNumber);
	fillFreeBlockVector(disk, freeBlockNumber);	
	char* indirectContent = (char*)calloc(blockSize, 1);
	
	for(int i = 10; i < arraySize; i++){
		memcpy(indirectContent+((i-10)*2), &blockNumberArray[i], 2);
	}

	writeBlock(disk, freeBlockNumber, indirectContent, blockSize);
	short temp	= (short) freeBlockNumber; 
	memcpy(indirectBlockNumber, &temp, 2);
}


void indirectSize(FILE* disk, short indirectBlockNumber, short* size){		
	char* buffer = (char*)calloc(blockSize, 1);
    readBlock(disk, indirectBlockNumber, buffer, blockSize);
	short temp = 0;
	short tempBlockNum = 1;
	
	for(int i = 0; i <(blockSize/2) &&(tempBlockNum!= 0); i++){
		memcpy(&tempBlockNum, buffer +(i*2), 2);
		if(tempBlockNum > 0){
			temp++;
		}
	}
	temp = temp + 10;
	memcpy(size, &temp, 2);
}


void indirectArray(FILE* disk, short indirectBlockNumber, short* resultBlockNumber, short arraySize){		
	char* buffer = (char*)calloc(blockSize, 1);
    readBlock(disk, indirectBlockNumber, buffer, blockSize);
	for(int i = 10; i < arraySize; i++){
		memcpy(&resultBlockNumber[i], buffer +((i-10)*2), 2);
	}
}


void createFileInode(FILE* disk, int freeBlockNumber, short* blockNumberArray, int arraySize, int fileSize){
	char* inode = calloc(blockSize, 1);
	int flags = 1;
	short dataBlockArray[12];
	
	for(int i = 0; i < 12; i++){
		dataBlockArray[i] = -1;
		memcpy(inode+8+(i*2), &dataBlockArray[i], 2);
	}
	
	if(arraySize < 11){
		for(int i = 0; i < arraySize; i++){
			dataBlockArray[i] = blockNumberArray[i];
			memcpy(inode+8+(i*2), &dataBlockArray[i], 2);
		}
		
		memcpy(inode, &fileSize, 4);
		memcpy(inode+4, &flags, 4);
		
		short singleIndirect = -1; 
		short doubleIndirect = -1; 
		
		memcpy(inode+28, &singleIndirect, 2);
		memcpy(inode+30, &doubleIndirect, 2);	
		
		writeBlock(disk, freeBlockNumber, inode, blockSize);
	}
	else{

		for(int i = 0; i < 10; i++){
			dataBlockArray[i] = blockNumberArray[i];
			memcpy(inode+8+(i*2), &dataBlockArray[i], 2);
		}
		
		short indirectBlockNumber = -1;
		create_singleIndirect(disk, blockNumberArray, arraySize, &indirectBlockNumber);

		memcpy(inode, &fileSize, 4);
		memcpy(inode+4, &flags, 4);
		
		short singleIndirect = indirectBlockNumber; 
		short doubleIndirect = -1; 
		
		memcpy(inode+28, &singleIndirect, 2);
		memcpy(inode+30, &doubleIndirect, 2);	
		
		writeBlock(disk, freeBlockNumber, inode, blockSize);
		
	}
	free(inode);
}


void createFileSingleInode(FILE* disk, int freeBlockNumber, int directBlockNumber){

	char* inode = calloc(blockSize, 1);
	int fileSize = 0;
	int flags = 1;
	short dataBlockArray[12];
	
	for(int i = 0; i < 12; i++){
		dataBlockArray[i] = -1;
		memcpy(inode+8+(i*2), &dataBlockArray[i], 2);
	}

    short dataBlock = (short) directBlockNumber;
	
	memcpy(inode, &fileSize, 4);
	memcpy(inode+4, &flags, 4);
	memcpy(inode+8, &dataBlock, 2);

	writeBlock(disk, freeBlockNumber, inode, blockSize);
	
	free(inode);
}	

	
void createDirectoryInode(FILE* disk, int freeBlockNumber, int directBlockNumber){

	char* inode = calloc(blockSize, 1);
	int fileSize = 0;
	int flags = 2;
	short dataBlockArray[12];
	
	for(int i = 0; i < 12; i++){
		dataBlockArray[i] = -1;
		memcpy(inode+8+(i*2), &dataBlockArray[i], 2);
	}

    short dataBlock = (short) directBlockNumber;
	
	memcpy(inode, &fileSize, 4);
	memcpy(inode+4, &flags, 4);
	memcpy(inode+8, &dataBlock, 2);

	writeBlock(disk, freeBlockNumber, inode, blockSize);
	
	free(inode);
}	


void readInode(FILE* disk, int inodeBlockNumber, short* resultBlockNumber){
	char* buffer = (char*)calloc(32, 1);
    readBlock(disk, inodeBlockNumber, buffer, 32);

	for(int i = 0; i < 12; i++){
		memcpy(&resultBlockNumber[i], buffer +(8 + 2*i), 2);
	}
}


void createDirectoryBlock(FILE* disk, int freeBlockNumber2){
	char* data = (char*)calloc(blockSize, 1);
	writeBlock(disk, freeBlockNumber2, data, blockSize);
	free(data);
}


void extendParentDirectory(FILE* disk, int parentDirectoryBlockNumber, int* newBlockNumber){
	int freeBlockNumber;
	readFreeBlockVector(disk, &freeBlockNumber);
	createDirectoryBlock(disk, freeBlockNumber);
	fillFreeBlockVector(disk, freeBlockNumber);
	memcpy(newBlockNumber, &freeBlockNumber, 4);

	int latest_dataBlockArray;
	short resultBlockNumber[12];
	
	readInode(disk, parentDirectoryBlockNumber, resultBlockNumber);

	for(int i = 0; i < 10; i++){
		if(resultBlockNumber[i] != -1){
			latest_dataBlockArray = i;
		}
	}
	
	resultBlockNumber[latest_dataBlockArray+1] = (short) freeBlockNumber;
	char* inode = calloc(32, 1);
    readBlock(disk, parentDirectoryBlockNumber, inode, 32);				
	memcpy(inode+8+((latest_dataBlockArray+1)*2), &resultBlockNumber[latest_dataBlockArray+1], 2);
	writeBlock(disk, parentDirectoryBlockNumber, inode, 32);
	free(inode);
}


void editParentDirectory(FILE* disk, int parentDirectoryBlockNumber, int childDirectoryInodeNumber, char* childDirectoryName){
	int latest_dataBlockArray;
	short resultBlockNumber[12];
	readInode(disk, parentDirectoryBlockNumber, resultBlockNumber);

	for(int i = 0; i < 10; i++){
		if(resultBlockNumber[i] != -1){
			latest_dataBlockArray = i;
		}
	}
	
	char* buffer = (char*)calloc(blockSize, 1);	
	readBlock(disk, resultBlockNumber[latest_dataBlockArray], buffer, blockSize);  
	int entryNumber = -1; 
	
	for(int i = 0;(i < 16)&&(entryNumber == -1); i++){
		unsigned char temp; 
		memcpy(&temp, buffer +(i*32), 1); 
		int temp2 = (int) temp; 
		if(temp2 == 0){
			entryNumber = i;
		}
	}
	
	if(entryNumber == -1){
		
		printf("\nExtending directory block of parent\n");
		
		int newBlockNumber;
		extendParentDirectory(disk, parentDirectoryBlockNumber, &newBlockNumber);
		unsigned char inodeNumber = (char) childDirectoryInodeNumber;

		memcpy(buffer +(entryNumber*32), &inodeNumber, 1);
		strncpy(buffer +(entryNumber*32) +1, childDirectoryName, 31);

		writeBlock(disk, newBlockNumber, buffer, blockSize);
	}
	
	else{
		unsigned char inodeNumber = (char) childDirectoryInodeNumber;

		memcpy(buffer +(entryNumber*32), &inodeNumber, 1);
		strncpy(buffer +(entryNumber*32) +1, childDirectoryName, 31);

		writeBlock(disk, resultBlockNumber[latest_dataBlockArray], buffer, blockSize);
	}
	free(buffer);
}


void deleteEntryDirectoryBlock(FILE* disk, int parentDirectBlockNumber, int deleletFileInodeNumber){
	
	char* buffer = (char*)calloc(blockSize, 1);	
	readBlock(disk, parentDirectBlockNumber, buffer, blockSize);

	int entryNumber = -1;
	
	for(int i = 0;(i < 16)&&(entryNumber == -1); i++){
		
		unsigned char temp;
		memcpy(&temp, buffer +(i*32), 1);
		
		int temp2 = (int) temp;
		
		if(temp2 == deleletFileInodeNumber){
			entryNumber = i;
		}
	}

	char* temp_entry_buffer = (char*)calloc(32, 1);	
	memcpy(buffer +(entryNumber*32), temp_entry_buffer, 32);
	writeBlock(disk, parentDirectBlockNumber, buffer, blockSize);
	
	free(buffer);
}


void searchFileOrDirectory(FILE* disk, int directBlockNumber, char* fileName, int* findingInode){

	int entryNumber = -1;

	char* buffer = (char*)calloc(blockSize, 1);
	readBlock(disk, directBlockNumber, buffer, blockSize);
	
	for(int i = 0;(i < 16)&&(entryNumber == -1); i++){

		char* buffer2 = (char*)calloc(31, 1);	
		strncpy(buffer2, buffer +(i*32) + 1, 30);

		if(strncmp(buffer2, fileName, 30) == 0){
			
			entryNumber = i;
		}
		free(buffer2); 
	}
	
	if(entryNumber != -1){
		
		unsigned char tempInodeNumber;
		memcpy(&tempInodeNumber, buffer +(entryNumber*32), 1);

		int tempInodeNumber2 = (int) tempInodeNumber;
		memcpy(findingInode, &tempInodeNumber2, 4);
	
	} else {
		
		int tempInodeNumber2 = -1;
		memcpy(findingInode, &tempInodeNumber2, 4);
	}
	
	free(buffer);
	
}


void createFileBlock(FILE* disk, int freeBlockNumber, char* fileContent){
	writeBlock(disk, freeBlockNumber, fileContent, blockSize);
}


void createRoot(FILE* disk){
	int freeBlockNumber;
	readFreeBlockVector(disk, &freeBlockNumber);
	fillFreeBlockVector(disk, freeBlockNumber);

	int rootDirectBlockNumber = freeBlockNumber;     
	int freeBlockNumber2;
	readFreeBlockVector(disk, &freeBlockNumber2);
	createDirectoryInode(disk, freeBlockNumber2, rootDirectBlockNumber);
	fillFreeBlockVector(disk, freeBlockNumber2);
	
	int nextFreeInodeIndex2;
	findNextFreeInode(disk, &nextFreeInodeIndex2);
	
	addMapping(disk, nextFreeInodeIndex2, freeBlockNumber);
	rootInodeIndex = nextFreeInodeIndex2;
	updateSuperblock(disk);
}


void initLLFS(FILE* disk){
	createSuperblock(disk);
	createFreeBlockVector(disk);
	createRoot(disk);
	inodeNum++;
	updateSuperblock(disk);
}


void createSubDirectory(FILE* disk, int parentDirectoryBlockNumber, char* childDirectoryName){
	int freeBlockNumber;
	readFreeBlockVector(disk, &freeBlockNumber);
	createDirectoryBlock(disk, freeBlockNumber);
	fillFreeBlockVector(disk, freeBlockNumber);
	int subDirectBlockNumber = freeBlockNumber;  
	
	int freeBlockNumber2; 
	readFreeBlockVector(disk, &freeBlockNumber2);
	createDirectoryInode(disk, freeBlockNumber2, subDirectBlockNumber);
	fillFreeBlockVector(disk, freeBlockNumber2);
	
	int nextFreeInodeIndex;
	findNextFreeInode(disk, &nextFreeInodeIndex);

	addMapping(disk, nextFreeInodeIndex, freeBlockNumber2);
	
	inodeNum++;
	updateSuperblock(disk);

	// modify parent
	int childDirectoryInodeNumber = nextFreeInodeIndex;   
	editParentDirectory(disk, parentDirectoryBlockNumber, childDirectoryInodeNumber, childDirectoryName);
}


void createFile(FILE* disk, char* fileContent, int parentDirectoryInode, char* fileName){
	if(strlen(fileContent) < 512){
		int freeBlockNumber;
		char* fileContent = (char*)calloc(blockSize, 1);
		
		strncpy(fileContent, fileContent, strlen(fileContent));

		readFreeBlockVector(disk, &freeBlockNumber);
		createFileBlock(disk, freeBlockNumber, fileContent);
		fillFreeBlockVector(disk, freeBlockNumber);


		int fileBlockNumber = freeBlockNumber;   
		int freeBlockNumber2;

		readFreeBlockVector(disk, &freeBlockNumber2);
		createFileSingleInode(disk, freeBlockNumber2, fileBlockNumber);
		fillFreeBlockVector(disk, freeBlockNumber2);

		
		int nextFreeInodeIndex;
		
		findNextFreeInode(disk, &nextFreeInodeIndex);
		addMapping(disk, nextFreeInodeIndex, freeBlockNumber2);
		
		
		inodeNum++;
		updateSuperblock(disk);
		
		int file_inodeNum = nextFreeInodeIndex;   
		editParentDirectory(disk, parentDirectoryInode, file_inodeNum, fileName);
	}
	else {
		
		int blocksNeeded;
		
		if((strlen(fileContent) % 512) != 0){
			blocksNeeded = (strlen(fileContent))/512+1;
		}else{
			blocksNeeded = (strlen(fileContent))/512;
		}
			
		short store_block_array[ blocksNeeded ];
			
		for( int i = 0; i < blocksNeeded; i++){
			int freeBlockNumber;
			
			char* fileContent = (char*)calloc(blockSize, 1);
			strncpy(fileContent, fileContent +(i*512), 512);

			readFreeBlockVector(disk, &freeBlockNumber);
			createFileBlock(disk, freeBlockNumber, fileContent);
			fillFreeBlockVector(disk, freeBlockNumber);

			store_block_array[i] = (short) freeBlockNumber;
		}

		int arraySize = blocksNeeded;
		
		int freeBlockNumber2;
		int fileSize = strlen(fileContent);

		readFreeBlockVector(disk, &freeBlockNumber2);
		fillFreeBlockVector(disk, freeBlockNumber2);
		createFileInode(disk, freeBlockNumber2, store_block_array, arraySize, fileSize);


		int nextFreeInodeIndex;
		
		findNextFreeInode(disk, &nextFreeInodeIndex);
		addMapping(disk, nextFreeInodeIndex, freeBlockNumber2);

		inodeNum++;
		updateSuperblock(disk);

		int fileInodeNum = nextFreeInodeIndex;   
		editParentDirectory(disk, parentDirectoryInode, fileInodeNum, fileName);
	}
}


void createEmptyFile(FILE* disk, int parentDirectoryInode, char* fileName){

	int freeBlockNumber;
	char* fileContent = (char*)calloc(blockSize, 1);
	
	readFreeBlockVector(disk, &freeBlockNumber);
	createFileBlock(disk, freeBlockNumber, fileContent);
	fillFreeBlockVector(disk, freeBlockNumber);


	int fileBlockNumber = freeBlockNumber;   
	int freeBlockNumber2;

	readFreeBlockVector(disk, &freeBlockNumber2);
	createDirectoryInode(disk, freeBlockNumber2, fileBlockNumber);
	fillFreeBlockVector(disk, freeBlockNumber2);

	int nextFreeInodeIndex;
	
	findNextFreeInode(disk, &nextFreeInodeIndex);
	addMapping(disk, nextFreeInodeIndex, freeBlockNumber2);
	
	inodeNum++;
	updateSuperblock(disk);
	int fileInodeNum = nextFreeInodeIndex;   
	editParentDirectory(disk, parentDirectoryInode, fileInodeNum, fileName);
}


void openFile(FILE* disk, char* input){

	char* parentDirectoryName = (char*)calloc(31,1);	
	char* currFileName = (char*)calloc(31,1);	
	char* fakeCurrFileName = (char*)calloc(31,1);	
	
	const char s[2] = "/";

	fakeCurrFileName = strtok(input, s);
		
	strncpy(parentDirectoryName, currFileName, 31);
	strncpy(currFileName, fakeCurrFileName, 31);
	
	fakeCurrFileName = strtok(NULL, s);
	
	int savedParentInodeBlockNumber = -1;
	
	int newestRootInodeIndex = getRootInodeIndex(disk);
		
	int rootInodeBlockNumber = findMapping(disk, newestRootInodeIndex);

	short resultBlockNumber[12];
	readInode(disk, rootInodeBlockNumber, resultBlockNumber);
		
	savedParentInodeBlockNumber = rootInodeBlockNumber;	

	while(fakeCurrFileName != NULL) {

		strncpy(parentDirectoryName, currFileName, 31);
		strncpy(currFileName, fakeCurrFileName, 31);
		fakeCurrFileName = strtok(NULL, s);

		if(fakeCurrFileName == NULL){
			
			int checkInodeNum;		
			short resultBlockNumber[12];
	
			readInode(disk, savedParentInodeBlockNumber, resultBlockNumber);
			
			for(int i = 0; resultBlockNumber[i] != -1; i++){
			
				searchFileOrDirectory(disk, resultBlockNumber[i], currFileName, &checkInodeNum);
				
				if(checkInodeNum > 0){
					
					savedParentInodeBlockNumber = findMapping(disk, checkInodeNum);
				}
			}

			if(checkInodeNum < 1){
				printf("\nNo such directory or file in this dataBlockArray: '%s'\n", currFileName);
				break;
			}
			
			short resultBlockNumber2[12];
			readInode(disk, savedParentInodeBlockNumber, resultBlockNumber2);
			if(resultBlockNumber2[10] > 0){
				short size;
				indirectSize(disk, resultBlockNumber2[10], &size);
				
				short resultBlockNumber5[size];
				readInode(disk, savedParentInodeBlockNumber, resultBlockNumber5);	
				
				indirectArray(disk, resultBlockNumber2[10], resultBlockNumber5, size);
				
				printf("\nContent of the file: ");
				
				for(int i = 0;(resultBlockNumber5[i] != -1) &&(i<size); i++){
					
					char* buffer = (char*)calloc(blockSize,1);	
					readBlock(disk, (int)resultBlockNumber5[i], buffer, blockSize);
					printf("%s", buffer);
						
					free(buffer);
				}
				printf("\n");
			}
			else{
				printf("\nContent of the file: ");
				for(int i = 0;(resultBlockNumber2[i] != -1) &&(i<10); i++){
					char* buffer = (char*)calloc(blockSize,1);	
					readBlock(disk, (int)resultBlockNumber2[i], buffer, blockSize);
					printf("%s", buffer);
						
					free(buffer);
				}
				printf("\n");
			}
		}

		else {
			int checkInodeNum;		
			short resultBlockNumber3[12];
			readInode(disk, savedParentInodeBlockNumber, resultBlockNumber3);
			
			for(int i = 0; resultBlockNumber3[i] != -1; i++){
				searchFileOrDirectory(disk, resultBlockNumber3[i], currFileName, &checkInodeNum);
				if(checkInodeNum > 0){
					savedParentInodeBlockNumber = findMapping(disk, checkInodeNum);
				}
			}
			if(checkInodeNum < 1){
				printf("\nNo such directory in this dataBlockArray: '%s'\n", currFileName);
				break;
			}
		}
	}
	
	free(parentDirectoryName);
	free(currFileName);
	free(fakeCurrFileName);	
}


void makeDirectory(FILE* disk, char* input){
	
	char* parentDirectoryName = (char*)calloc(31,1);	
	char* currentDirectoryName = (char*)calloc(31,1);	
	char* fakeCurrentDirectoryName = (char*)calloc(31,1);	
	const char s[2] = "/";

	fakeCurrentDirectoryName = strtok(input, s); // skip "Mkdir"

	strncpy(parentDirectoryName, currentDirectoryName, 31);
	strncpy(currentDirectoryName, fakeCurrentDirectoryName, 31);
	
	fakeCurrentDirectoryName = strtok(NULL, s);
	int savedParentInodeBlockNumber = -1;
	int newestRootInodeIndex = getRootInodeIndex(disk);
	int rootInodeBlockNumber = findMapping(disk, newestRootInodeIndex);
	short resultBlockNumber4[12];
	readInode(disk, rootInodeBlockNumber, resultBlockNumber4);
	savedParentInodeBlockNumber = rootInodeBlockNumber;
	
	while(fakeCurrentDirectoryName != NULL) {
		strncpy(parentDirectoryName, currentDirectoryName, 31);
		strncpy(currentDirectoryName, fakeCurrentDirectoryName, 31);
		fakeCurrentDirectoryName = strtok(NULL, s);
		printf("currentDirectoryName: %s ", currentDirectoryName);
		printf("parentDirectoryName: %s\n", parentDirectoryName);
		if(fakeCurrentDirectoryName == NULL){
			createSubDirectory(disk, savedParentInodeBlockNumber, currentDirectoryName);
			printf("\nWe added a sub directory: %s\n", currentDirectoryName);
		}
		else{
			int checkInodeNum;		
			short resultBlockNumber[12];
			readInode(disk, savedParentInodeBlockNumber, resultBlockNumber);

			for(int i = 0; resultBlockNumber[i] != -1; i++){
				searchFileOrDirectory(disk, resultBlockNumber[i], currentDirectoryName, &checkInodeNum);
				if(checkInodeNum > 0){
					savedParentInodeBlockNumber = findMapping(disk, checkInodeNum);
				}
			}
			
			if(checkInodeNum < 1){
				printf("No such directory in this dataBlockArray: '%s'\n", currentDirectoryName);
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
	char* currFileName = (char*)calloc(31,1);	
	char* fakeCurrentDirectoryName = (char*)calloc(31,1);	
	const char s[2] = "/";
	fakeCurrentDirectoryName = strtok(input, s);
	strncpy(parentDirectoryName, currFileName, 31);
	strncpy(currFileName, fakeCurrentDirectoryName, 31);
	fakeCurrentDirectoryName = strtok(NULL, s);
	int savedParentInodeBlockNumber = -1;
	int newestRootInodeIndex = getRootInodeIndex(disk);
	int rootInodeBlockNumber = findMapping(disk, newestRootInodeIndex);
	short resultBlockNumber4[12];
	readInode(disk, rootInodeBlockNumber, resultBlockNumber4);
	savedParentInodeBlockNumber = rootInodeBlockNumber;
	
	while(fakeCurrentDirectoryName != NULL) {
		strncpy(parentDirectoryName, currFileName, 31);
		strncpy(currFileName, fakeCurrentDirectoryName, 31);
		fakeCurrentDirectoryName = strtok(NULL, s);
		if(fakeCurrentDirectoryName == NULL){
			createFile(disk, fileContent, savedParentInodeBlockNumber, currFileName);
			printf("\nWe added a file, file name: %s\n", currFileName);
		}
		else{
			int checkInodeNum;		
			short resultBlockNumber[12];
			readInode(disk, savedParentInodeBlockNumber, resultBlockNumber);
			for(int i = 0; resultBlockNumber[i] != -1; i++){
				searchFileOrDirectory(disk, resultBlockNumber[i], currFileName, &checkInodeNum);
				if(checkInodeNum > 0){
					savedParentInodeBlockNumber = findMapping(disk, checkInodeNum);
				}
			}
			if(checkInodeNum < 1){
				printf("\nNo such directory in this dataBlockArray: '%s'\n", currFileName);
				break;
			}
		}
	}
	
	free(parentDirectoryName);
	free(currFileName);
	free(fakeCurrentDirectoryName);	
}



void writeEmptyFile(FILE* disk, char* input){
	
	char* parentDirectoryName = (char*)calloc(31,1);	
	char* currFileName = (char*)calloc(31,1);	
	char* fakeCurrentDirectoryName = (char*)calloc(31,1);	
	const char s[2] = "/";
	
	fakeCurrentDirectoryName = strtok(input, s); // skip the first "Mkfile"
	strncpy(parentDirectoryName, currFileName, 31);
	strncpy(currFileName, fakeCurrentDirectoryName, 31);
	fakeCurrentDirectoryName = strtok(NULL, s);
	
	int savedParentInodeBlockNumber = -1;
	int newestRootInodeIndex = getRootInodeIndex(disk);
	int rootInodeBlockNumber = findMapping(disk, newestRootInodeIndex);
	short resultBlockNumber4[12];
	
	readInode(disk, rootInodeBlockNumber, resultBlockNumber4);
	savedParentInodeBlockNumber = rootInodeBlockNumber;
	
	while(fakeCurrentDirectoryName != NULL) {
		strncpy(parentDirectoryName, currFileName, 31);
		strncpy(currFileName, fakeCurrentDirectoryName, 31);
		fakeCurrentDirectoryName = strtok(NULL, s);
		
		if(fakeCurrentDirectoryName == NULL){
			createEmptyFile(disk, savedParentInodeBlockNumber, currFileName);
			printf("\nWe added an empty file, file name: %s\n", currFileName);
		} else{
			int checkInodeNum;		
			short resultBlockNumber[12];
			readInode(disk, savedParentInodeBlockNumber, resultBlockNumber);
			
			for(int i = 0; resultBlockNumber[i] != -1; i++){
				searchFileOrDirectory(disk, resultBlockNumber[i], currFileName, &checkInodeNum);
				if(checkInodeNum > 0){
					savedParentInodeBlockNumber = findMapping(disk, checkInodeNum);
				}
			}
			if(checkInodeNum < 1){
				printf("\nNo such directory in this dataBlockArray: '%s'\n", currFileName);
				break;
			}
		}
	}
	
	free(parentDirectoryName);
	free(currFileName);
	free(fakeCurrentDirectoryName);	
}


void deleteFile(FILE* disk, int parentDirectBlockNumber, char* currFileName, int deleletFileInodeNumber){

	short blockNumberToDelete[12];
	int deleteInodeBlockNumber = findMapping(disk, deleletFileInodeNumber);
	readInode(disk, deleteInodeBlockNumber, blockNumberToDelete);

	for(int i = 0; blockNumberToDelete[i] != -1; i++){
		deleteFreeBlockVector(disk, blockNumberToDelete[i]);
	}
	
	deleteFreeBlockVector(disk, deleteInodeBlockNumber);
	inodeNum--;
	updateSuperblock(disk);
	deleteMapping(disk, deleletFileInodeNumber);
	deleteEntryDirectoryBlock(disk, parentDirectBlockNumber, deleletFileInodeNumber);
}


void deleteDirectory(FILE* disk, int parentDirectBlockNumber, char* currFileName, int del_dir_inodeNum){
	
	short blockNumberToDelete[12];
	int deleteInodeBlockNumber = findMapping(disk, del_dir_inodeNum);
	
	readInode(disk, deleteInodeBlockNumber, blockNumberToDelete);
	
	for(int i = 0; blockNumberToDelete[i] != -1; i++){
		deleteFreeBlockVector(disk, blockNumberToDelete[i]);
	}
	
	deleteFreeBlockVector(disk, deleteInodeBlockNumber);
	inodeNum--;
	updateSuperblock(disk);
	deleteMapping(disk, del_dir_inodeNum);
	deleteEntryDirectoryBlock(disk, parentDirectBlockNumber, del_dir_inodeNum);
}


void rmFile(FILE* disk, char* input){
	
	char* parentDirectoryName = (char*)calloc(31,1);	
	char* currFileName = (char*)calloc(31,1);	
	char* fakeCurrentDirectoryName = (char*)calloc(31,1);	
	const char s[2] = "/";

	fakeCurrentDirectoryName = strtok(input, s);		// skip the first command "Mkfile"
	strncpy(parentDirectoryName, currFileName, 31);
	strncpy(currFileName, fakeCurrentDirectoryName, 31);
	fakeCurrentDirectoryName = strtok(NULL, s);
	
	int savedParentInodeBlockNumber = -1;
	int newestRootInodeIndex = getRootInodeIndex(disk);
	int rootInodeBlockNumber = findMapping(disk, newestRootInodeIndex);
	short resultBlockNumber4[12];
	
	readInode(disk, rootInodeBlockNumber, resultBlockNumber4);
	savedParentInodeBlockNumber = rootInodeBlockNumber;
	
	while(fakeCurrentDirectoryName != NULL) {

		strncpy(parentDirectoryName, currFileName, 31);
		strncpy(currFileName, fakeCurrentDirectoryName, 31);
		fakeCurrentDirectoryName = strtok(NULL, s);

		if(fakeCurrentDirectoryName == NULL){
			short resultBlockNumber[12];
			readInode(disk, savedParentInodeBlockNumber, resultBlockNumber);
			int checkInodeNum = -1;		
			int fileBlockDirectory;
			
			for(int i = 0;(resultBlockNumber[i] != -1) &&(checkInodeNum == -1); i++){
				searchFileOrDirectory(disk, resultBlockNumber[i], currFileName, &checkInodeNum);
				if(checkInodeNum != -1){
					fileBlockDirectory = i;
				}
			}
			
			if(checkInodeNum == -1){
				printf("File does not exist\n");
				break;
			}
			
			deleteFile(disk, resultBlockNumber[fileBlockDirectory], currFileName, checkInodeNum);
			printf("\nWe deleted a file, file name: %s\n", currFileName);
		} else {
			int checkInodeNum;		
			short resultBlockNumber[12];
			readInode(disk, savedParentInodeBlockNumber, resultBlockNumber);
			
			for(int i = 0; resultBlockNumber[i] != -1; i++){
				searchFileOrDirectory(disk, resultBlockNumber[i], currFileName, &checkInodeNum);
				if(checkInodeNum > 0){
					savedParentInodeBlockNumber = findMapping(disk, checkInodeNum);
				}
			}
			
			if(checkInodeNum < 1){
				printf("\nNo such directory in this dataBlockArray: '%s'\n", currFileName);
				break;
			}
		}
	}
	free(parentDirectoryName);
	free(currFileName);
	free(fakeCurrentDirectoryName);	
}

void listFile(FILE* disk, char* input){
	char* parentDirectoryName = (char*)calloc(31,1);	
	char* currentDirectoryName = (char*)calloc(31,1);	
	char* fakeCurrentDirectoryName = (char*)calloc(31,1);	
	const char s[2] = "/";

	fakeCurrentDirectoryName = strtok(input, s); // skip the first "Mkfile"
	strncpy(parentDirectoryName, currentDirectoryName, 31);
	strncpy(currentDirectoryName, fakeCurrentDirectoryName, 31);
	fakeCurrentDirectoryName = strtok(NULL, s);

	int savedParentInodeBlockNumber = -1;
	int newestRootInodeIndex = getRootInodeIndex(disk);
	int rootInodeBlockNumber = findMapping(disk, newestRootInodeIndex);
	short resultBlockNumber4[12];
	
	readInode(disk, rootInodeBlockNumber, resultBlockNumber4);
	savedParentInodeBlockNumber = rootInodeBlockNumber;

	if(fakeCurrentDirectoryName == NULL){
			short resultBlockNumber[12];
			readInode(disk, savedParentInodeBlockNumber, resultBlockNumber);	

			printf("\nHere is all the files in %s directory: \n", currentDirectoryName);

			for(int i = 0;(resultBlockNumber[i] != -1) &&(i<10); i++){
				short inodeNum = 0;
				char* buffer = (char*)calloc(blockSize, 1);
				readBlock(disk, resultBlockNumber[i], buffer, blockSize);

				for(int i = 0; i < 16; i++){
					char* buffer2 = (char*)calloc(31, 1);
					strncpy(buffer2, buffer +(i*32) + 1, 31);
					memcpy(&inodeNum, buffer +(i*32), 1);
					
					if(inodeNum != 0) {
						printf("File name %d : %s\n", i, buffer);
					}
					free(buffer2); 
				}
				free(buffer);
			}
			printf("\nWe listed all the files in %s directory\n", currentDirectoryName);
	}

	while(fakeCurrentDirectoryName != NULL) {

		strncpy(parentDirectoryName, currentDirectoryName, 31);
		strncpy(currentDirectoryName, fakeCurrentDirectoryName, 31);
		fakeCurrentDirectoryName = strtok(NULL, s);

		if(fakeCurrentDirectoryName == NULL){
			int checkInodeNum;		
			short resultBlockNumber2[12];
			readInode(disk, savedParentInodeBlockNumber, resultBlockNumber2);
			
			for(int i = 0; resultBlockNumber2[i] != -1; i++){
				searchFileOrDirectory(disk, resultBlockNumber2[i], currentDirectoryName, &checkInodeNum);
				if(checkInodeNum > 0){
					savedParentInodeBlockNumber = findMapping(disk, checkInodeNum);
				}
			}
			if(checkInodeNum < 1){
				printf("\nNo such directory in this dataBlockArray: '%s'\n", currentDirectoryName);
				break;
			}
			
			short resultBlockNumber[12];
			readInode(disk, savedParentInodeBlockNumber, resultBlockNumber);	

			printf("\nHere is all the files in %s directory:\n", currentDirectoryName);

			for(int i = 0;(resultBlockNumber[i] != -1) &&(i<10); i++){

				short inodeNum = 0;
				char* buffer = (char*)calloc(blockSize, 1);
				readBlock(disk, resultBlockNumber[i], buffer, blockSize);

				for(int i = 0; i < 16; i++){
					char* buffer2 = (char*)calloc(31, 1);
					strncpy(buffer2, buffer +(i*32) + 1, 31);
					memcpy(&inodeNum, buffer +(i*32), 1);
						
					if(inodeNum != 0) {
						
						printf("File name %d : %s\n", i, buffer);
					}
					
					free(buffer2); 
				}
				free(buffer);
			}
			printf("\nWe listed all the files in %s directory.\n", currentDirectoryName);
		} else {
			int checkInodeNum;		
			short resultBlockNumber3[12];
			readInode(disk, savedParentInodeBlockNumber, resultBlockNumber3);
			
			for(int i = 0; resultBlockNumber3[i] != -1; i++){
				searchFileOrDirectory(disk, resultBlockNumber3[i], currentDirectoryName, &checkInodeNum);
				if(checkInodeNum > 0){
					savedParentInodeBlockNumber = findMapping(disk, checkInodeNum);
				}
			}
			
			if(checkInodeNum < 1){
				printf("\nNo such directory in this dataBlockArray: '%s'\n", currentDirectoryName);
				break;
			}
		}
	}
	
	free(parentDirectoryName);
	free(currentDirectoryName);
	free(fakeCurrentDirectoryName);	
}



void rmDirectory(FILE* disk, char* input){
	
	char* parentDirectoryName = (char*)calloc(31,1);	
	char* currentDirectoryName = (char*)calloc(31,1);	
	char* fakeCurrentDirectoryName = (char*)calloc(31,1);	
	const char s[2] = "/";
	
	fakeCurrentDirectoryName = strtok(input, s);   // skip the first command "Mkfile"
	strncpy(parentDirectoryName, currentDirectoryName, 31);
	strncpy(currentDirectoryName, fakeCurrentDirectoryName, 31);
	fakeCurrentDirectoryName = strtok(NULL, s);
	
	int savedParentInodeBlockNumber = -1;
	int newestRootInodeIndex = getRootInodeIndex(disk);
	int rootInodeBlockNumber = findMapping(disk, newestRootInodeIndex);
	short resultBlockNumber4[12];
	
	readInode(disk, rootInodeBlockNumber, resultBlockNumber4);
	savedParentInodeBlockNumber = rootInodeBlockNumber;
	
	while(fakeCurrentDirectoryName != NULL) {
		strncpy(parentDirectoryName, currentDirectoryName, 31);
		strncpy(currentDirectoryName, fakeCurrentDirectoryName, 31);
		fakeCurrentDirectoryName = strtok(NULL, s);
		if(fakeCurrentDirectoryName == NULL){
			short resultBlockNumber[12];
			readInode(disk, savedParentInodeBlockNumber, resultBlockNumber);
			int checkInodeNum = -1;		
			int fileBlockDirectory;
			
			for(int i = 0;(resultBlockNumber[i] != -1) &&(checkInodeNum == -1); i++){
				searchFileOrDirectory(disk, resultBlockNumber[i], currentDirectoryName, &checkInodeNum);
				if(checkInodeNum != -1){
					fileBlockDirectory = i;
				}
			}
			if(checkInodeNum == -1){
				printf("\nNo such directory: '%s'\n", currentDirectoryName);
				break;
			}
			
			deleteDirectory(disk, resultBlockNumber[fileBlockDirectory], currentDirectoryName, checkInodeNum);
			printf("\nWe deleted a directory, directory name: %s\n", currentDirectoryName);
		} else {
			int checkInodeNum;		
			short resultBlockNumber[12];
			readInode(disk, savedParentInodeBlockNumber, resultBlockNumber);
			for(int i = 0; resultBlockNumber[i] != -1; i++){
				searchFileOrDirectory(disk, resultBlockNumber[i], currentDirectoryName, &checkInodeNum);
				if(checkInodeNum > 0){
					savedParentInodeBlockNumber = findMapping(disk, checkInodeNum);
				}
			}
			
			if(checkInodeNum < 1){
				printf("\nNo such directory in this dataBlockArray: '%s'\n", currentDirectoryName);
				break;
			}
		}
	}
	
	free(parentDirectoryName);
	free(currentDirectoryName);
	free(fakeCurrentDirectoryName);	
}

void commandInput(FILE* disk, char* input, char* fileContent){

	char stringTok[10];
	strncpy(stringTok, input, 6);
	char* command = (char*)calloc(60,1);	
	char s[2] = "/";
	command = strtok(stringTok, s);		// skip the first command "open"
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

	if(strncmp(command, "list", 4) == 0){		
		printf("Listing the file in a directory at '%s'", path);
		listFile(disk, input);
		
	} else if (strncmp(command, "Open", 4) == 0){
		printf("Opening a file at '%s'", path);
		openFile(disk, input);
		
	} else if(strncmp(command, "Rmdir", 5) == 0){		
		printf("Deleting a directory at '%s'", path);
		rmDirectory(disk, input);
		
	} else if(strncmp(command, "Rmfile", 6) == 0){
		printf("Deleting a file at '%s'", path);
		rmFile(disk, input);
		
	} else if((strncmp(command, "Writefile", 6) == 0) && fileContent == NULL){
		printf("Making a new empty file at '%s'", path);
		writeEmptyFile(disk, input);
		
	} else if(strncmp(command, "Writefile", 6) == 0){			
		printf("Making a new file with content at '%s'", path);
		writeFile(disk, input, fileContent);
		
	} else if(strncmp(command, "Mkdir", 5) == 0){		
		printf("Making a directory at '%s'", path);
		makeDirectory(disk, input);
	}
}