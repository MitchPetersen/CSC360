#include <stdio.h>
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>


void func(int arr[], int val) {
	
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
	float numbers[4000];
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
	return 0;
}