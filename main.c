#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

struct Process {
    char id;
    int arrive;
    int burst_time;
    int burst_num;
    int io;
};

void msg_error(char *msg) {
    fprintf(stderr, "ERROR: %s\n", msg);
    exit(EXIT_FAILURE);
}

void msg_event(int t, char *msg) {
    printf("time %ims: %s\n", t, msg);
    fflush(stdout);
}

void msg_event_q_i(int t, char id, char *msg, char *msg2, int i, struct Process *ready, int n) {
    printf("time %ims: ", t);
    if (id != ' ') printf("Process %c ", id); // Assume a process can't have an id of ' '
    printf("%s ", msg);
    if (strcmp(msg2, "") != 0) printf("%i %s ", i, msg2);
    printf("[Q ");
    if (n == 0) printf("<empty>");
    else {
        int i;
        bool first = true;
        for (i = 0; i < n; i++) {
            if (first) first = false;
            else printf(" ");
            printf("%c", ready[i].id);
        }
    }
    printf("]\n");
    fflush(stdout);
}

void msg_event_q(int t, char id, char *msg, struct Process *ready, int n) {
    msg_event_q_i(t, id, msg, "", 0, ready, n);
}

void first_come_first_serve(struct Process *array, int arrsize, int t_cs, int *turnaround_time, int *burst_time, int *wait_time){
    int i;
    *turnaround_time = 0;
    *burst_time = 0;
    *wait_time = 0;
    //Working under the assumption that they come are in the file sorted by arrival time, can change later if that's not the case
    for(i = 0; i < arrsize; i++){
        //If I/O time needs to be calculated
        if(array[i].burst_num > 0){
            // burst_num * burst_times for time spent bursting, and then add the time spent in io between each one
            *turnaround_time += (array[i].burst_num * array[i].burst_time) + (array[i].io * array[i].burst_num);
            // Just the burst times
            *burst_time += (array[i].burst_num * array[i].burst_time);
        }
        else{
            //As above, but only once
            *turnaround_time += array[i].burst_time;
            *burst_time += array[i].burst_time;
        }
        *wait_time += t_cs;
        *turnaround_time += t_cs;
        //Add context switch time
    }
    //Average all the times
    *burst_time /= arrsize;
    *turnaround_time /= arrsize;
    *wait_time /= arrsize;
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

int main(int argc, char * argv[]) {
    // Open File
    FILE *input = fopen(argv[1], "r");
    if (input == NULL) msg_error("Invalid input file format");
    
    // Simulation Configuration
    int n = 0; // number of processes to simulate; will be determined via input file
    int m = 1; // number of processors (i.e. cores) available w/in the CPU
    int t_cs = 6; // time (in ms) it takes to perform a context switch
    int t_slice = 94; // time slice (in ms) for RR
    
    // Get all lines from file
    char line[100];
    int size_array = 3;
    char **array_raw = (char**) calloc(size_array, sizeof(char*));
    while (fgets(line, sizeof(line), input) != NULL) {
        if (line[0] != '#') {
            if (n >= size_array) {
                size_array = size_array * 2;
                array_raw = realloc(array_raw, size_array * sizeof(char*));
                if (array_raw == NULL) msg_error("memory for array_raw not re-allocated");
            }
            array_raw[n] = malloc(strlen(line) + 1);
            strcpy(array_raw[n], line);
            n += 1;
        }
    }
    fclose(input);
    
    // Create Processes
    struct Process *array = (struct Process*) calloc(n - 1, sizeof(struct Process));
    int i;
    int j;
    int init;
    for (i = 0; i < n; i++) {
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
    
    // First Come First Serve (FCFS)
    int t = 0;
    int ready_n = 0;
    struct Process *ready = (struct Process*) calloc(ready_n, sizeof(struct Process));
    int waiting_n = 0;
    struct Process *waiting = (struct Process*) calloc(waiting_n, sizeof(struct Process));
    msg_event_q(t, ' ', "Simulator started for FCFS", ready, ready_n);
    for (i = 0; i < n; i++) {
        if (array[i].arrive <= t) {
            ready_n++;
            ready = realloc(ready, ready_n * sizeof(struct Process*));
            if (ready == NULL) msg_error("memory for ready not re-allocated");
            ready[ready_n - 1] = array[i];
            msg_event_q(t, ready[ready_n - 1].id, "arrived and added to ready queue", ready, ready_n);
        }
        else {
            waiting_n++;
            waiting = realloc(waiting, waiting_n * sizeof(struct Process*));
            if (waiting == NULL) msg_error("memory for waiting not re-allocated");
            waiting[waiting_n - 1] = array[i];
        }
    }
    int ready_max = ready_n;
    int waiting_max = waiting_n;
    struct Process running;
    while (ready_n > 0) {
        running = ready[0];
        for (i = 1; i < ready_n; i++) ready[i - 1] = ready[i];
        ready_n--;
        t += t_cs/2;
        msg_event_q(t, running.id, "started using the CPU", ready, ready_n);
        while (running.burst_num > 0) {
            t += running.burst_time;
            running.burst_num--;
            msg_event_q_i(t, running.id, "completed a CPU burst;", "bursts to go", running.burst_num, ready, ready_n);
        }
        // TBA: currently, bursts until finished; needs to burst once, then be added to blocked, re-added later
        // TBA: continue simulation, add waiting, etc
    }
    msg_event(t, "Simulator ended for FCFS");
    
    // TBA: Shortest Remaining Time (SRT)
    
    // TBA: Round Robin (RR)
    
    // TBA: output file stuff, don't forget to check if file can be opened (same process/error as w/ source file)
    
    // Problem: can't just free(array), dunno how to deallocate it
    exit(EXIT_SUCCESS);
}