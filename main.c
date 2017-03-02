#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void msg_error(char *msg) {
    fprintf(stderr, "ERROR: %s\n", msg);
    exit(EXIT_FAILURE);
}

void msg_event(int t, char *msg) {
    printf("time %ims: %s\n", t, msg);
    fflush(stdout);
}

struct Process {
    char id;
    int arrive;
    int burst_time;
    int burst_num;
    int io;
};

void first_come_first_serve(struct Process *array, int arrsize, int t_cs, int turnaround_time, int burst_time, int wait_time){
    int i;
    turnaround_time = 0;
    burst_time = 0;
    wait_time = 0;
    //Working under the assumption that they come are in the file sorted by arrival time, can change later if that's not the case
    for(i = 0; i < arrsize; i++){
        //If I/O time needs to be calculated
        if(array[i].burst_num > 0){
            // burst_num * burst_times for time spent bursting, and then add the time spent in io between each one
            turnaround_time += (array[i].burst_num * array[i].burst_time) + (array[i].io * array[i].burst_num);
            // Just the burst times
            burst_time += (array[i].burst_num * array[i].burst_time);
        }
        else{
            //As above, but only once
            turnaround_time += array[i].burst_time;
            burst_time += array[i].burst_time;
        }
        wait_time += t_cs;
        turnaround_time += t_cs;
        //Add context switch time
    }
    //Average all the times
    burst_time /= arrsize;
    turnaround_time /= arrsize;
    wait_time /= arrsize;
}

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
    FILE *input = fopen(argv[1], "r"); // for some reason, 1 = main.c, 2 = filename?
    if (input == NULL) msg_error("Invalid input file format");
    
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
    
    // Create Processes
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
    
    // Print out results (TBA: remove)
    for (i = 0; i < count; i++) {
        printf("Process %c - arrive: %i, burst time: %i, burst num: %i, io: %i.\n", array[i].id, array[i].arrive, array[i].burst_time, array[i].burst_num, array[i].io);
    }
    fflush(stdout);
    
    // First Come First Serve (FCFS)
    int t = 0;
    //msg_event(t, "Simulator started for FCFS [Q <empty>]");
    // TBA - probably should isolate in sub-functions
    //msg_event(t, "Simulator ended for FCFS");
    
    // Shortest Remaining Time (SRT)
    //msg_event(t, "Simulator started for SRT [Q <empty>]");
    // TBA - probably should isolate in sub-functions
    //msg_event(t, "Simulator ended for SRT");
    
    // Round Robin (RR)
    //msg_event(t, "Simulator started for RR [Q <empty>]");
    // TBA - probably should isolate in sub-functions
    //msg_event(t, "Simulator ended for RR");
    
    // TBA: output file stuff, don't forget to check if file can be opened (same process/error as w/ source file)
    
    // Problem: can't just free(array), dunno how to deallocate it
    exit(EXIT_SUCCESS);
}