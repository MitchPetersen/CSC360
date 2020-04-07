// Mitch Petersen V00845204

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../io/File.c"


int main(int argc, char* argv[]) {
	FILE* disk;	
	if(fopen("../disk/vdisk", "r") == NULL){
		printf("\n We can't find the old vdisk file, Making a new one\n");
		disk = fopen("../disk/vdisk", "wb+");
		char* init = calloc(BLOCK_SIZE*NUM_BLOCKS, 1);
		fwrite(init, BLOCK_SIZE*NUM_BLOCKS, 1, disk);
		free(init);
	} else {
		disk = fopen("../disk/vdisk", "rb+"); // create the file to be written to in binary mode
	}
	
	initLLFS(disk);
	readSuperblock(disk);
	
	//test case: "Mkdir /momsspaghetti"
	char input1[] = "Mkdir /momsspaghetti";
	command_input(disk, input1, input1);
	
	//test case: "Mkdir /momsspaghetti/m&m"
	char input2[] = "Mkdir /momsspaghetti/m&m";
	command_input(disk, input2, input2);

	//test case: "Mkdir /momsspaghetti/m&m/wow"
	char input3[] = "Mkdir /momsspaghetti/m&m/wow";
	command_input(disk, input3, input3);
	
	//test case: Write file /momsspaghetti/m&m/wow/reality1" and the content is "m&m world"
	char input4[] = "Writefile /momsspaghetti/m&m/wow/reality1";
	char file_content[] = "m&m world";
	command_input(disk, input4, file_content);
	
	//test case: "Open /momsspaghetti/m&m/wow/reality1"
	char input5[] = "Open /momsspaghetti/m&m/wow/reality1";
	command_input(disk, input5, input5);
	
	//test case: "Mkdir /momsspaghetti/snap"
	char input6[] = "Mkdir /momsspaghetti/snap";
	command_input(disk, input6, input6);
	
	//test case: "Writefile /momsspaghetti/m&m/wow/reality2" and the content is "m&m world m&m world"
	char input7[] = "Writefile //momsspaghetti/m&m/wow/reality2";
	char fileContent[] = "m&m world m&m world";
	command_input(disk, input7, fileContent);

	//test case: file larger than 1 block	
	//test case: "Writefile /momsspaghetti/snap/reality3" and the content is 512*a + 512*b + 2*c
	char input8[] = "Writefile /momsspaghetti/snap/reality3";
	char* fileContent1 =(char*)calloc(1025, 1);
	
	for(int i = 0; i < 512; i++){
		fileContent1[i] = 'm';
	}
	for(int i = 512; i < 1024; i++){
		fileContent1[i] = '&';
	}
	for(int i = 1024; i < 1025; i++){
		fileContent1[i] = 'm';
	}
	command_input(disk, input8, fileContent1);

	
	//test case: "Open /momsspaghetti/snap/reality3"
	char input9[] = "Open /momsspaghetti/snap/reality3";
	command_input(disk, input9, input9);
	
	//test case: delete(Rmfile)
	char input10[] = "Rmfile /momsspaghetti/snap/reality3";
	command_input(disk, input10, input10);

	//test case: "list /momsspaghetti/snap/reality3"
	char input11[] = "list /momsspaghetti/snap";
	command_input(disk, input11, input11);
	
	//test case: delete directory
	char input12[] = "Rmdir /momsspaghetti/snap";
	command_input(disk, input12, input12);

	//test case: "list /momsspaghetti/snap"
	char input13[] = "list /momsspaghetti/snap";
	command_input(disk, input13, input13);
	
	//test case: "list /momsspaghetti"
	char input14[] = "list /momsspaghetti";
	command_input(disk, input14, input14);	
	
	//test case: file create in root dir
	char input15[] = "Writefile /thereGoesGravity";
	char fileContent2[] = "m&m world m&m world";
	command_input(disk, input15, fileContent2);
	
	//test case: "list /"
	char input16[] = "list /";
	command_input(disk, input16, input16);	
	
	//test case: "Writefile /momsspaghetti/m&m/wow/theresShaunHannity" and the content is Empty
	char input17[] = "Writefile /momsspaghetti/m&m/wow/theresShaunHannity";
	char* fileContent3 = NULL;
	command_input(disk, input17, fileContent3);
	
	//test case: "list /momsspaghetti/m&m/wow"
	char input18[] = "list /momsspaghetti/m&m/wow";
	command_input(disk, input18, input18);	
	
	//test case: "Open /momsspaghetti/m&m/wow/theresShaunHannity"
	char input19[] = "Open /momsspaghetti/m&m/wow/theresShaunHannity";
	command_input(disk, input19, input19);
	
	//test case: 
	char input20[] = "Writefile /momsspaghetti/m&m/wow/loseYourself";
	char blam[] = "Look If you had One shot Or one opportunity To seize everything you ever wanted In one moment Would you capture it Or just let it slip? Yo His palms are sweaty, knees weak, arms are heavy There's vomit on his sweater already, mom's spaghetti He's nervous, but on the surface he looks calm and ready To drop bombs, but he keeps on forgettin' What he wrote down, the whole crowd goes so loud He opens his mouth, but the words won't come out He's chokin', how, everybody's jokin' now The clocks run out, times up, over, blaow! Snap back to reality, oh there goes gravity Oh, there goes Rabbit, he choked He's so mad, but he won't give up that easy? No He won't have it, he knows his whole back's to these ropes It don't matter, he's dope, he knows that, but he's broke He's so stagnant, he knows, when he goes back to this mobile home, that's when its Back to the lab again yo, this whole rhapsody Better go capture this moment and hope it don't pass him You better lose yourself in the music, the moment You own it, you better never let it go You only get one shot, do not miss your chance to blow This opportunity comes once in a lifetime You better lose yourself in the music, the moment You own it, you better never let it go You only get one shot, do not miss your chance to blow This opportunity comes once in a lifetime You better His soul's escaping, through this hole that is gaping This world is mine for the taking Make me king, as we move toward a New World Order A normal life is borin', but super stardom's close to post mortar It only grows harder, only grows hotter He blows, it's all over, these hoes is all on him Coast to coast shows, he's known as the Globetrotter Lonely roads, God only knows, he's grown farther from home, he's no father He goes home and barely knows his own daughter But hold your nose 'cause here goes the cold water These hoes don't want him no mo', he's cold product They moved on to the next schmo who flows, he nose dove and sold nada So the soap opera is told and unfolds, I suppose it's old partna, but the beat goes on Da-da-dum, da-dum, da-da You better lose yourself in the music, the moment You own it, you better never let it go You only get one shot, do not miss your chance to blow This opportunity comes once in a lifetime You better lose yourself in the music, the moment You own it, you better never let it go You only get one shot, do not miss your chance to blow This opportunity comes once in a lifetime You better No more games, I'ma change what you call rage Tear this motherfuckin' roof off like two dogs caged I was playin' in the beginnin', the mood all changed I been chewed up and spit out and booed off stage But I kept rhymin' and stepped right in the next cypher Best believe somebody's payin' the Pied Piper All the pain inside amplified by the Fact that I can't get by with my nine to Five and I can't provide the right type of Life for my family 'cause man, these goddamn food stamps don't buy diapers And its no movie, there's no Mekhi Phifer This is my life and these times are so hard And it's getting even harder tryna feed and water my seed, plus Teeter-totter, caught up between bein' a father and a prima-donna Baby mama drama, screamin' on her, too much For me to wanna stay in one spot, another day of monotony's Gotten me to the point, I'm like a snail I've got To formulate a plot fore I end up in jail or shot Success is my only motherfuckin' option, failure's not Mom, I love you, but this trailer's got to go, I cannot grow old in Salem's lot So here I go, is my shot Feet, fail me not, this may be the only opportunity that I got You better lose yourself in the music, the moment You own it, you better never let it go You only get one shot, do not miss your chance to blow This opportunity comes once in a lifetime You better lose yourself in the music, the moment You own it, you better never let it go You only get one shot, do not miss your chance to blow This opportunity comes once in a lifetime You better You can do anything you set your mind to, man";
	command_input(disk, input20, blam);
	
	//test case: "Open /momsspaghetti/m&m/wow/loseYourself"
	char input21[] = "Open /momsspaghetti/m&m/wow/loseYourself";
	command_input(disk, input21, input21);

	
	printf("\n	# Finished");
	
	fclose(disk);
    return 0;
}