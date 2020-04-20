/**
 * This program forks a separate process using the fork()/exec() system calls.
 *
 * Figure 3.08
 *
 * @author Silberschatz, Galvin, and Gagne
 * Operating System Concepts  - Tenth Edition
 * Copyright John Wiley & Sons - 2018
 */

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <pthread.h> 

void *mythread() {
	execlp("/bin/ls","ls",NULL);
}

int main()
{
	pthread_t th_id;
	pthread_create(&th_id, NULL, mythread, NULL);
	pthread_join(th_id, NULL);
	
    return 0;
}