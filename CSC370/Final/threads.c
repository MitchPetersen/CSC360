#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <pthread.h> 

void *myread() {
	execlp("/bin/ls","ls",NULL);
}

int main()
{
	pthread_t th_id;
	pthread_create(&th_id, NULL, myread, NULL);
	pthread_join(th_id, NULL);
	
    return 0;
}