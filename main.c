#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void msg_error(char *msg) {
    fprintf(stderr, "ERROR: <%s>\n", msg);
    exit(EXIT_FAILURE);
}

struct Process {
    char id;
    int arrive;
    int burst_time;
    int burst_num;
    int io;
};

int next(int *j, char *array_raw) {
    int init = *j;
    char line[100];
    while (array_raw[*j] != '|') {
        line[*j - init] = array_raw[*j];
        line[*j - init + 1] = '\0';
        *j = *j + 1;
    }
    *j = *j + 1;
    return atoi(line);
}

int main( int argc, char * argv[]) {
    // Open File
    FILE *input = fopen(argv[2], "r"); // for some reason, 1 = main.c, 2 = filename?
    if (input == NULL) msg_error("could not read file");
    
    // Get all lines from file
    char line[100];
    int size_array = 3;
    int count = 0;
    char **array_raw = (char**) calloc(size_array, sizeof(char*));
    while (fgets(line, sizeof(line), input) != NULL) {
        if (line[0] != '#') {
            if (count >= size_array) {
                size_array = size_array * 2;
                array_raw = realloc(array_raw, size_array * sizeof(char*));
                if (array_raw == NULL) msg_error("memory for array_raw not re-allocated");
            }
            array_raw[count] = malloc(strlen(line) + 1);
            strcpy(array_raw[count], line);
            count += 1;
        }
    }
    fclose(input);
    
    // Seperate lines into different arrays based on data type
    struct Process *array = (struct Process*) calloc(count - 1, sizeof(struct Process));
    int i;
    int j;
    int init;
    for (i = 0; i < count; i++) {
        array[i].id = array_raw[i][0];
        j = 2;
        array[i].arrive = next(&j, array_raw[i]);
        array[i].burst_time = next(&j, array_raw[i]);
        array[i].burst_num = next(&j, array_raw[i]);
        // Problem: when trying to set io with next, *** stack smashing detected ***
        init = j;
        strcpy(line, "");
        while (array_raw[i][j] != '|' && j < strlen(array_raw[i])) {
            line[j - init] = array_raw[i][j];
            line[j - init + 1] = '\0';
            j++;
        }
        array[i].io = atoi(line);
        free(array_raw[i]);
    }
    free(array_raw);
    
    // Print out results
    for (i = 0; i < count; i++) {
        printf("Process %c - arrive: %i, burst time: %i, burst num: %i, io: %i.\n", array[i].id, array[i].arrive, array[i].burst_time, array[i].burst_num, array[i].io);
    }
    fflush(stdout);
    
    // Problem: can't just free(array), dunno how to deallocate it
    
    exit(EXIT_SUCCESS);
}