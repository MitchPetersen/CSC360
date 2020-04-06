// Mitch Petersen V00845204

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../io/File.c"

int main(int argc, char* argv[]) {
	FILE* disk;	
	if ( fopen("../disk/vdisk", "r") == NULL ){
		
		printf( "\nWe can't find the old vdisk file, creating a new one\n");
		
		disk = fopen("../disk/vdisk", "wb+"); // Open the file to be written to in binary mode
	
		char* init = calloc(blockSize*numberBlocks, 1);
		fwrite(init, blockSize*numberBlocks, 1, disk);
		free(init);
	}
	else {
		disk = fopen("../disk/vdisk", "rb+"); // create the file to be written to in binary mode
		char* init = calloc(blockSize*numberBlocks, 1);
		fwrite(init, blockSize*numberBlocks, 1, disk);
		free(init);
	}

	initLLFS( disk );
	
	fclose(disk);
    return 0;
}