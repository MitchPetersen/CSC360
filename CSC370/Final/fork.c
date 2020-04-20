/***************************************************************************//**
  @file         processes.c
  @author       Mitch Petersen, V00845204
*******************************************************************************/

#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h> 

int main(int argc, char **argv) {
	FILE *out = fopen("output.txt", "w");

	fork();
	fork();
	fork();

	pid_t pid = getpid();
	pid_t ppid = getppid();
	
	fprintf(out, "this process is %u and it's parent is %u\n", pid, ppid);
}
