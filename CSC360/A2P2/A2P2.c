#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <float.h>

extern char* strdup(const char*);

float numbers[4000];
int datapoint_size=0;
int num_threads = 4;

typedef struct line {
	float intercept;
	float slope;
	float SAR;
} line;

line bestline[4];

line slope_calc(float y1, float y2, float x1, float x2){
	line line_obj;
	line_obj.slope = (y2-y1)/(x2-x1);
	line_obj.intercept=y1-(line_obj.slope*x1);
	return line_obj;
}

float dist(line line_obj, float y, float x){
	float p2 = (line_obj.slope*x)+line_obj.intercept;
	float dist = y-p2;
	dist = fabsf(dist);
	return dist;
}

void *func(void* i) {
	int index = *(int *) i;
	float minimum_distance = FLT_MAX;
	for(int i = index; i<datapoint_size; i+=num_threads){
		for(int j = index+1; j<datapoint_size; j++){
			line line_obj;
			line_obj = slope_calc(numbers[i], numbers[j], (float) i, (float) j);
			float total_distance=0;
			for(int k=0; k<datapoint_size; k++){
				total_distance += dist(line_obj, numbers[k], k);
			}
			printf("SAR: %f  Slope: %f  intercept: %f\n", total_distance, line_obj.slope, line_obj.intercept);
			if (total_distance<minimum_distance){
				bestline[i] = line_obj;
				minimum_distance = total_distance;
				bestline[i].SAR=minimum_distance;
				
			}
		}
	}
	return NULL;
}

char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

int main(int argc, char** argv) {
	FILE *input;
	input = fopen(argv[1], "r");
	
	char** tokens;
	if (input == NULL) {
		fprintf(stderr, "problem reading file\n");
		exit(1);
	}
	char lines[256];
	int linenum = 0;
	while(fgets(lines, sizeof(lines), input)) {
		if (linenum==0) {
			linenum++;
		} else {
			tokens = str_split(lines, ',');
			numbers[linenum-1] = strtof(tokens[1], NULL);
			//printf("%f\n", numbers[linenum-1]);
			linenum++;
			
			datapoint_size+=1;
		}
	}
	fclose(input);
	//printf("datapoint: %d\n", datapoint_size);
	pthread_t thread_arr[num_threads];
	int num_array[num_threads];
	for (int k=0; k<num_threads; k++){
		num_array[k]=k;
	}
	for (int i=0; i<num_threads; i++){
		pthread_create(&thread_arr[i], NULL, func, &num_array[i]);
	}
	for (int i=0; i<num_threads; i++){
		pthread_join(thread_arr[i], NULL);
	}
	line best = bestline[0];
	float sar = FLT_MAX;
	for (int i=0; i<num_threads; i++){
		if(bestline[i].SAR<sar){
			sar = bestline[i].SAR;
			best = bestline[i];
		}
	}
	printf("Best Line:\nSlope: %f\nIntercept: %f\nSAR: %f\n", best.slope, best.intercept, best.SAR);
	
	return 0;
}