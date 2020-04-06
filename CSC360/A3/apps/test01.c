
// Ronald Liu V00838627

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../io/File.c"


int main(int argc, char* argv[]) {
	
FILE* disk;	

	if (fopen("../disk/vdisk", "r") == NULL){
		
		printf("\n we can't find the old vdisk file, so we will create new one\n");
		printf(" -------------------------------------------------------------------- \n\n");
		
		disk = fopen("../disk/vdisk", "wb+"); // Open the file to be written to in binary mode
	
		char* init = calloc(blockSize*numberBlocks, 1);
		fwrite(init, blockSize*numberBlocks, 1, disk);
		free(init);
	}
	else {
		
		disk = fopen("../disk/vdisk", "rb+"); // create the file to be written to in binary mode

	}
	initLLFS(disk);

	readSuperblock (disk);
	
	//test case: "Mkdir /home"
	char input1[] = "Mkdir /home";
	commandInput (disk, input1, input1);
	
	//test case: "Mkdir /home/dir1"
	char input2[] = "Mkdir /home/dir1";
	commandInput (disk, input2, input2);

	//test case: "Mkdir /home/dir1/dir2"
	char input3[] = "Mkdir /home/dir1/dir2";
	commandInput (disk, input3, input3);
	
	
	//test case: "Writefile /home/dir1/dir2/helloworld" with content "hello world"
	
	char input4[] = "Writefile /home/dir1/dir2/helloworld";
	char fileContent1[] = "hello world";
	commandInput (disk, input4, file_content);

	
	
	//test case: "Open /home/dir1/dir2/file1"
	char input5[] = "Open /home/dir1/dir2/file1";
	commandInput (disk, input5, input5);

	
	//test case: "Mkdir /home/dir1mk2"
	char input6[] = "Mkdir /home/dir1mk2";
	commandInput (disk, input6, input6);
	
	
	//test case: "Writefile /home/dir1/dir2/helloworldmk2" and the content is "hello world hello world"
	
	char input7[] = "Writefile //home/dir1/dir2/helloworldmk2";
	char fileContent2[] = "hello world hello world";
	commandInput (disk, input7, file_content_larger2);
	
	
	//test case: file larger than 1 block	
	//test case: "Writefile /home/dir1mk2/testing" and the content is 512 x's + 512 y's + 1 z's
	char input8[] = "Writefile /home/dir1mk2/testing";
	char* fileContent3 = (char*)calloc(1025, 1);
	
	for (int i = 0; i < 512; i++){
		fileContent3[i] = 'x';
	}
	for (int i = 512; i < 1024; i++){
		fileContent3[i] = 'y';
	}
	for (int i = 1024; i < 1025; i++){
		fileContent3[i] = 'z';
	}
	commandInput (disk, input8, fileContent3);
	
	//test case: "Open /home/dir1mk2/testing"
	char input9[] = "Open /home/dir1mk2/testing";
	commandInput (disk, input9, input9);
	
	//test case: delete (Rmfile)
	char input10[] = "Rmfile /home/dir1mk2/testing";
	commandInput (disk, input10, input10);


	//test case: "list /home/dir1mk2/testing"
	char input11[] = "list /home/dir1mk2";
	commandInput (disk, input11, input11);
	
	
	//test case: delete (Rmdir)
	char input12[] = "Rmdir /home/dir1mk2";
	commandInput (disk, input12, input12);

	
	//test case: "list /home/dir1mk2"
	char input13[] = "list /home/dir1mk2";
	commandInput (disk, input13, input13);
	
	
	//test case: "list /home"
	char input14[] = "list /home";
	commandInput (disk, input14, input14);	
	
	
	//test case: file create in root dir
	char input15[] = "Writefile /helloworldmk3";
	char fileContent4[] = "hello world hello world";
	commandInput (disk, input15, fileContent4);
	
	
	//test case: "list /"
	char input16[] = "list /";
	commandInput (disk, input16, input16);	
	
	
	//test case: "Writefile /home/dir1/dir2/empty" and the content is Empty
	
	char input17[] = "Writefile /home/dir1/dir2/empty";
	char* fileContent5 = NULL;
	commandInput (disk, input17, fileContent5);
	
	//test case: "list /home/dir1/dir2"
	char input18[] = "list /home/dir1/dir2";
	commandInput (disk, input18, input18);	
	
	//test case: "Open /home/dir1/dir2/empty"
	char input19[] = "Open /home/dir1/dir2/empty";
	commandInput (disk, input19, input19);
	
	
	//test case: 
	char input20[] = "Writefile /home/dir1/dir2/bigfinale";
	char* fileContent6 = (char*)calloc(4015, 1);
	strcpy(fileContent6, "Look, if you had mom's spaghetti, Would you capture it or just let it slip? Yo His palms spaghetti, knees weak, arms spaghetti There's vomit on his sweater spaghetti, mom's spaghetti He's nervous, but on the surface he looks calm spaghetti to drop bombs, But he keeps on spaghetti what he wrote down, The whole crowd goes spaghetti He opens his mouth, but spaghetti won't come out He's choking how, everybody's joking now Spaghetti run out, time's up, over, bloah! Snap back to spaghetti, Oh there goes spaghetti Oh, there goes spaghetti, bloah He's so mad, but he won't give up spaghetti, no. He won't have it, he knows he keeps on forgetting That mom's spaghetti's dope He knows that but he's broke He's so stagnant, he knows When he goes back to his mom's spaghetti, that's when it's Back to the lab again, yo This whole spaghetti He better go capture spaghetti and hope it don't pass him You better lose yourself in mom's spaghetti, it's ready You better never let it go(go) You only get one spaghetti, do not miss your chance to blow Cause spaghetti comes once in a lifetime yo You better lose yourself in mom's spaghetti, it's ready You better never let it go(go) You only get one spaghetti, do not miss your chance to blow Cause spaghetti comes once in a lifetime yo (You better) The soul's escaping, through this hole that is gaping Mom's spaghetti's mine for the taking Make me spaghetti, as we move toward a new world order A normal sweater is boring, but mom's spaghetti's close to post mortem It only grows harder, spaghetti grows hotter He vomits all over. spaghetti's all on him Coast to coast shows, he blows his own daughter He only grows harder, only grows hotter He goes home and barely knows his own mom's spaghetti There's vomit on his mom's spaghetti His hoes don't want him no more, he's cold spaghetti They moved on to the next schmoe who flows man He knows his palms are sweaty ope hes calm and ready ope And unfolds I suppose it's old spaghetti Chewed up and spit out he's chokin now You better lose yourself in mom's spaghetti, it's ready You better never let it go(go) You only get one spaghetti, do not miss your chance to blow Cause spaghetti comes once in a lifetime yo You better lose yourself in mom's spaghetti, it's ready You better never let it go(go) You only get one spaghetti, do not miss your chance to blow Cause spaghetti comes once in a lifetime yo (You better) No more games, I'ma change what you call spaghetti Tear this motherfucking roof off like two mom's spaghettis I was playing in the beginning, the mood all changed spaghetti Chewed up and spit out and there's vomit on his sweater But I kept chewin and stepped right into the next cypher Hold your nose cause here goes the damn diaper All the vomit inside amplified by the fact That I keep on forgetting to make spaghetti And I can't provide the right type of spaghetti for my family Cause man, these goddamn food stamps don't buy spaghetti And it's no movie, there's no mom's spaghetti, this is my life And these palms are so sweaty, and i'm so hard My seed's escaping through this hole that is gaping Caught up between being a father and a prima donna Baby vomit's on his sweater already Mom's spaghetti he's nervous Bloah another day of monotony Has gotten me to the point, I'm like a mom I've got to formulate spaghetti or I end up in jail or shot Spaghetti is my only motherfucking option, vomit's not Mom, I love you, but this vomit's got to go Ope ope, led de dome de dome led de dome de dome So here I go it's my shot. This may be the only mom's spaghetti that I got You better lose yourself in mom's spaghetti, it's ready You better never let it go(go) You only get one spaghetti, do not miss your chance to blow Cause spaghetti comes once in a lifetime yo You better lose yourself in mom's spaghetti, it's ready You better never let it go(go) You only get one spaghetti, do not miss your chance to blow Cause spaghetti comes once in a lifetime yo (You better) Mom's Spaghetti You can do anything man"
	
	printf(fileContent6)
	
	commandInput (disk, input20, fileContent6);
	
	
	//test case: "Open /home/dir1/dir2/bigfinale"
	char input21[] = "Open /home/dir1/dir2/bigfinale";
	commandInput (disk, input21, input21);

	
	printf("\n        **********END OF TEST CASES***********  \n\n\n");
	
	fclose(disk);
    return 0;
}




