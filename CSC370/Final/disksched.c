#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <pthread.h> 
#include <dirent.h>



int main()
{
	int arr[15] = {19, 73, 2, 15, 16, 180, 12, 65, 34, 67, 34, 82, 13, 42, 102};
	
	printf("FCFS, start position 0\n");
	int seek = 0;
	for (int i=0; i<15; i++) {
		printf("Servicing %i\n", arr[i]);
		if(i>=1) {
			seek += abs(arr[i]-arr[i-1]);
		} else {
			seek += arr[i];
		}
	}
	printf("Seek time of %i\n\n", seek);
	
	printf("SSTF, start position 0\n");
	seek = 0;
	int arr[15] = {2,12,13,15,16,19,34,34,42,65,67,73,82,102,180};
	for (int j=0; j<15; j++) {
		printf("Servicing %i\n", arr[j]);
		if(j>=1) {
			seek += abs(arr[j]-arr[j-1]);
		} else {
			seek += arr[j];
		}
	}
	printf("Seek time of %i\n\n", seek);
}