
// Ronald Liu V00838627

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../io/File.c"


int main(int argc, char* argv[]) {
	
FILE* disk;	

	if ( fopen("../disk/vdisk", "r") == NULL ){
		
		printf( "\n we can't find the old vdisk file, so we will create new one\n");
		printf( " -------------------------------------------------------------------- \n\n" );
		
		disk = fopen("../disk/vdisk", "wb+"); // Open the file to be written to in binary mode
	
		char* init = calloc(blockSize*numberBlocks, 1);
		fwrite(init, blockSize*numberBlocks, 1, disk);
		free(init);
	}
	else {
		
		disk = fopen("../disk/vdisk", "rb+"); // create the file to be written to in binary mode

	}
	//initLLFS( disk );

	readSuperblock ( disk );
	
	
	
	
	//test case: "Mkdir /home"
	char input1[] = "Mkdir /home";
	commandInput ( disk, input1, input1 );
	
	//test case: "Mkdir /home/hello"
	char input2[] = "Mkdir /home/hello";
	commandInput ( disk, input2, input2 );

	//test case: "Mkdir /home/hello/hello2"
	char input3[] = "Mkdir /home/hello/hello2";
	commandInput ( disk, input3, input3 );
	
	
	//test case: "Writefile /home/hello/hello2/helloworld" with content "hello world"
	
	char input4[] = "Writefile /home/hello/hello2/helloworld";
	char file_content[] = "hello world";
	commandInput ( disk, input4, file_content );

	
	
	//test case: "Open /home/hello/hello2/file1"
	char input5[] = "Open /home/hello/hello2/file1";
	commandInput ( disk, input5, input5 );

	
	//test case: "Mkdir /home/hello_2nd"
	char input6[] = "Mkdir /home/hello_2nd";
	commandInput ( disk, input6, input6 );
	
	
	//test case: "Writefile /home/hello/hello2/testing2" and the content is "hello world hello world"
	
	char input7[] = "Writefile //home/hello/hello2/testing2";
	char file_content_larger2[] = "hello world hello world";
	commandInput ( disk, input7, file_content_larger2 );

	

	//test case: file larger than 1 block	
	//test case: "Writefile /home/hello_2nd/testing" and the content is 512*a + 512*b + 2*c
	char input8[] = "Writefile /home/hello_2nd/testing";
	char* file_content_larger = (char*)calloc( 1026, 1 );
	
	for ( int i = 0; i < 512; i++ ){
		file_content_larger[i] = 'a';
	}
	for ( int i = 512; i < 1024; i++ ){
		file_content_larger[i] = 'b';
	}
	for ( int i = 1024; i < 1026; i++ ){
		file_content_larger[i] = 'c';
	}
	commandInput ( disk, input8, file_content_larger );

	
	//test case: "Open /home/hello_2nd/testing"
	char input9[] = "Open /home/hello_2nd/testing";
	commandInput ( disk, input9, input9 );
	
	//printf( "\n\n\n\n" );
	//read_superblock( disk );	
	
	//test case: delete ( Rmfile )
	char input10[] = "Rmfile /home/hello_2nd/testing";
	commandInput ( disk, input10, input10 );


	//test case: "list /home/hello_2nd/testing"
	char input11[] = "list /home/hello_2nd";
	commandInput ( disk, input11, input11 );
	
	
	//test case: delete ( Rmdir )
	char input12[] = "Rmdir /home/hello_2nd";
	commandInput ( disk, input12, input12 );

	
	//test case: "list /home/hello_2nd"
	char input13[] = "list /home/hello_2nd";
	commandInput ( disk, input13, input13 );
	
	
	
	//test case: "list /home"
	char input14[] = "list /home";
	commandInput ( disk, input14, input14 );	
	
	
	//test case: file create in root dir
	char input15[] = "Writefile /testing3";
	char file_content_larger3[] = "hello world hello world";
	commandInput ( disk, input15, file_content_larger3 );
	
	
	//test case: "list /"
	char input16[] = "list /";
	commandInput ( disk, input16, input16 );	
	
	
	//test case: "Writefile /home/hello/hello2/testing4" and the content is Empty
	
	char input17[] = "Writefile /home/hello/hello2/testing4";
	char* file_content_empty = NULL;
	commandInput ( disk, input17, file_content_empty );
	
	//test case: "list /home/hello/hello2"
	char input18[] = "list /home/hello/hello2";
	commandInput ( disk, input18, input18 );	
	
	//test case: "Open /home/hello/hello2/testing4"
	char input19[] = "Open /home/hello/hello2/testing4";
	commandInput ( disk, input19, input19 );
	
	
	//test case: 
	char input20[] = "Writefile /home/hello/hello2/test11";
	char* file_content_larger4 = (char*)calloc( 9141, 1 );
	
	for ( int i = 0; i < 5120; i++ ){
		file_content_larger4[i] = 'a'; }
	for ( int i = 5120; i < 9140; i++ ){
		file_content_larger4[i] = 'b'; }
	file_content_larger4[9140] = 'u';
	
	//printf( "\n\n%s\n\n" , file_content_larger4 );
	commandInput ( disk, input20, file_content_larger4 );
	
	
	//test case: "Open /home/hello/hello2/test11"
	char input21[] = "Open /home/hello/hello2/test11";
	commandInput ( disk, input21, input21 );

	
	printf( "\n        **********END OF TEST CASES***********  \n\n\n" );
	
	fclose(disk);
    return 0;
}




