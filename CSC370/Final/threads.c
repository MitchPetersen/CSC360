#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <pthread.h> 
#include <dirent.h>

struct dirent *readdir(DIR *dirp);

void *myread() {
	printf("in thread\n");
	struct dirent *de;
	DIR *dir = opendir("."); 
	if (dir == NULL)
    { 
        printf("Couldn't open current directory" ); 
        return 0; 
    }
	while ((de = readdir(dir)) != NULL) 
            printf("%s ", de->d_name); 
		
	closedir(dir);
}

int main()
{
	printf("before thread\n");
	pthread_t th_id;
	pthread_create(&th_id, NULL, myread, NULL);
	pthread_join(th_id, NULL);
	printf("\nafter thread\n");
    return 0;
}