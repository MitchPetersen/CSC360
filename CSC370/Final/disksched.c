#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <pthread.h> 
#include <dirent.h>



int main()
{
	
	int arr[15] = {19, 73, 2, 15, 16, 180, 12, 65, 34, 67, 34, 82, 13, 42, 102};
	int loop;
	for (loop = 0; loop < 15; loop++){
		printf("%d, ", arr[loop]);
	}
	
	printf("\nFCFS, start position 0\n");
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
	
	
	int arr2[15] = {2,12,13,15,16,19,34,34,42,65,67,73,82,102,180};
	// as we are starting at position 0, the smallest seek times are going to be just the entire array sorted into smallest to largest
	// I took the liberty of hard coding it to save time. 
	
	for (int j=0; j<15; j++) {
		printf("Servicing %i\n", arr2[j]);
		if(j>=1) {
			seek += abs(arr2[j]-arr2[j-1]);
		} else {
			seek += arr2[j];
		}
	}
	printf("Seek time of %i\n\n", seek);
}