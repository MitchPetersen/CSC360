#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <pthread.h> 
#include <dirent.h>



int main()
{
	int arr[15] = {19, 73, 2, 15, 16, 180, 12, 65, 34, 67, 34, 82, 13, 42, 102};
	
	printf("FCFS, start position 0");
	int seek = 0;
	for (int i=0; i<15; i++) {
		printf("Servicing %i\n", arr[i]);
		if(i>=1) {
			seek += abs(arr[i]-arr[i-1]);
		} else {
			seek += arr[i];
		}
	}
	
}