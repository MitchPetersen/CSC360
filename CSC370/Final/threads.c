#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <pthread.h> 
#include <dirent.h>

struct dirent *readdir(DIR *dirp);

void *myread() {
	printf(readdir());
}

int main()
{
	printf("before thread\n");
	pthread_t th_id;
	pthread_create(&th_id, NULL, myread, NULL);
	pthread_join(th_id, NULL);
	printf("after thread\n");
    return 0;
}