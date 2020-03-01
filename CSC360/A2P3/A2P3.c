#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <unistd.h>
#include <semaphore.h>

int phil_id[5];
sem_t chops[5];

void get_max_min(int* min, int* max){
	min = min<max?min:max;
	max = min>max?min:max;
}

void* eat(void* id){
	printf("here");
	int index = *(int*) id;
	int min = index;
	int max = index+1%5;
	get_max_min(&min, &max);
	while(1) {
		printf("%d, thinking...", index);
		sleep(1);
		printf("%d, hungry....", index);
		sem_wait(&chops[min]);
		sem_wait(&chops[max]);
		printf("%d, eating.... nom", index);
		sem_post(&chops[max]);
		sem_post(&chops[min]);
	}
}

int main(int argc, char** argv) {
	pthread_t philosophers[5];
	
	for(int i=0; i<5; i++){
		phil_id[i]=i;
		sem_init(&chops[i], 0, 1);
	}
	printf("1");
	for(int i=0; i<5; i++){
		pthread_create(&philosophers[i], NULL, eat, &phil_id[i]);
	}
	printf("2");
	for(int i=0; i<5; i++){
		pthread_join(philosophers[i], NULL);
	}
}