#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
extern char* strdup(const char*);

float numbers[4000];
int datapoint_size;
int num_threads = 4;

struct line {
	float intercept;
	float slope;
	float SAR;
} line;

line bestline[4];

float slope_calc(float y1, float y2, float x1, float x2){
	float slope = (y2-y1)/(x2-x1);
	return slope;
}

float intercept_calc(float slope, float x, float y){
	float intercept = y-(slope*x);
	return intercept;
}

float dist(line line_obj, float y, float x){
	float p2 = (line_obj.slope*x)+line_obj.intercept;
	float dist = p2-y;
	dist = fabs(dist);
	return dist;
}

void *func(void* i) {
	int index = *(int *) i;
	float minimum_distance = FLT_MAX;
	datapoint_size = sizeof(numbers)/sizeof(float);
	for(int i = index; i<datapoint_size; i+num_threads){
		for(int j = index+1; j<datapoint_size; j++){
			line line_obj;
			line_obj.slope = slope_calc(numbers[i], numbers[j], i, j);
			line_obj.intercept = intercept_calc(line_obj.slope, i, j);
			float total_distance=0;
			for(int k=0; k<datapoint_size; k++){
				total_distance += dist(line_obj, numbers[k], k);
			}
			if (total_distance<minimum_distance){
				minimum_distance = total_distance;
				line_obj.SAR=minimum_distance;
				bestline[i] = line_obj;
			}
		}
	}
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
	char line[256];
	int linenum = 0;
	while(fgets(line, sizeof(line), input)) {
		if (linenum==0) {
			linenum++;
		} else {
			tokens = str_split(line, ',');
			numbers[linenum-1] = atof(tokens[1]);
			printf("%f\n", numbers[linenum-1]);
			linenum++;
		}
	}
	fclose(input);
	pthread_t thread_arr[num_threads];
	for (int i=0; i<num_threads; i++){
		pthread_create(&thread_arr[i], NULL, func, &i);
	}
	for (int i=0; i<num_threads; i++){
		pthread_join(thread_arr[i], NULL);
	}
	line best;
	float sar = FLT_MAX;
	for (int i=0; i<num_threads; i++){
		if(bestline[i].SAR<sar){
			sar = bestline[i].sar;
			best = bestline[i];
		}
	}
	printf("Best Line:\nSlope: %f\nIntercept: %f\nSAR: %f\n", best.slope, best.intercept, best.SAR);
	
	return 0;
}