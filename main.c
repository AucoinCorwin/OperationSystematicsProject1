#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "msg.h"

void srt_add(struct Process *array, struct Process proc, int *arrsize){
    int i;
    struct Process temp;
    arrsize++;
    array[*arrsize - 1] = proc;
    for(i = 0; i < *arrsize; i++){
        int j;
        while(j > 0 && array[j - 1].burst_time > array[j].burst_time){
            temp = array[j];
            array[j] = array[j - 1];
            array[j - 1] = temp;
            j -= 1;
        }
    }
    // Just insertion sort by burst_time for srt, shortest burst_time should be at [0] after every call
    // Right now the array comes in with the free space (add 1 to arrsize before passing it)
    // Could probably change that to happen in the function so we have some wiggle room with error checking
    // We should only need to do this when each process arrives for the first time and every time something comes back from block
    // Also we need to check afterwords to see if it has a shorter time than the thing in the running thing, and if it does we need to preempt it
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
    struct Process *array = (struct Process*) calloc(n, sizeof(struct Process));
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
    struct Process *ready = (struct Process*) calloc(n, sizeof(struct Process));
    int waiting_n = 0;
    struct Process *waiting = (struct Process*) calloc(n, sizeof(struct Process));
    msg_event_q(t, ' ', "Simulator started for FCFS", ready, ready_n);
    for (i = 0; i < n; i++) {
        if (array[i].arrive <= t) {
            ready_n++;
            ready[ready_n - 1] = array[i];
            msg_event_q(t, ready[ready_n - 1].id, "arrived and added to ready queue", ready, ready_n);
        }
        else {
            waiting_n++;
            waiting[waiting_n - 1] = array[i];
        }
    }
    struct Process running;
    struct Process *blocked = (struct Process*) calloc(n, sizeof(struct Process));
    int blocked_n = 0;
    while (ready_n > 0) {
        running = ready[0]; // Add the first process in queue to running
        for (i = 1; i < ready_n; i++) ready[i - 1] = ready[i];
        ready_n--;
        t += t_cs/2;
        
        // TBA: Send a message when a process arrives, also we need to figure this out for SRT, as it can cause preemptions
        msg_event_q(t, running.id, "started using the CPU", ready, ready_n);
        while (running.burst_num > 0) {
            t += 1;
            running.burst_left--;
            if (running.burst_left < 1) {
                msg_event_q_i(t, running.id, "completed a CPU burst;", "bursts to go", running.burst_num - 1, ready, ready_n);
                if (running.burst_num > 0) { // Needs to be added to blocked
                    blocked_n++;
                    blocked[blocked_n - 1] = running;
                    // Somehow set it up so that the io knows when it's done
                    if (ready_n == 0) break; // Break if here's nothing left to run
                    running = ready[0];
                    running.burst_left = running.burst_time;
                    for (i = 1; i < ready_n; i++) ready[i - 1] = ready[i];
                    ready_n--;
                    t += t_cs; // Context switch time
                }
                else { // Leave the gun, take the cannoli
                    if (ready_n == 0) break;
                    running = ready[0]; // Set running to the next available process
                    running.burst_left = running.burst_time;
                    for(i = 1; i < ready_n; i++) ready[i - 1] = ready[i];
                    ready_n--;
                    t += t_cs; // Context switch time
                }
            }
            int j;
            for (j = 1; j < blocked_n; j++) { // Check which blocked items have finished their I/O stuff while CPU burst was happening
                blocked[j - 1].io_left -= 1;
                if (blocked[j - 1].io_left < 0) { // It finished I/O, let's put it back in the ready queue
                    if (ready_n == 0) break;
                    // TBA: Message should be sent here, but Darien has this weird function for building messages so she can do that
                    ready_n++;
                    ready[ready_n - 1] = blocked[j - 1]; // Add it back to the ready queue (for FCFS, we just put it at the end)
                    int h;
                    for (h = j; h < blocked_n; h++) blocked[h - 1] = blocked[h]; // Remove it from the blocked list (It's not really a queue)
                    blocked_n--;
                }
            }
            for (j = 1; j < ready_n; j++) ready[ready_n - 1].wait_time += 1;
            for (j = 1; j < waiting_n; j++) {
                if (waiting[j - 1].arrive < t) { // Should've arrived in between those starbursts
                    ready_n++;
                    ready[ready_n - 1] = waiting[j - 1]; // Add it to ready queue
                    // TBA: Message should be sent here, but Darien has this weird function for building messages so she can do that
                    int h;
                    for (h = j; h < waiting_n; h++) waiting[h - 1] = waiting[h]; // Remove it from waiting queue
                    waiting_n--;
                }
            }
            msg_event_q_i(t, running.id, "completed a CPU burst;", "bursts to go", running.burst_num, ready, ready_n);
            if(running.burst_num > 0) { // Needs to be added to blocked
                blocked_n++;
                blocked[blocked_n - 1] = running;
                // Somehow set it up so that the io knows when it's done
                if(ready_n == 0) break; // There's nothing left to run
                running = ready[0];
                running.burst_left = running.burst_time;
                for (i = 1; i < ready_n; i++) ready[i - 1] = ready[i];
                ready_n--;
                t += t_cs; // Context switch time
                
            }
            else { // Leave the gun, take the cannoli
                if (ready_n == 0) break;
                running = ready[0]; //Set running to the next available process
                running.burst_left = running.burst_time;
                for (i = 1; i < ready_n; i++) ready[i - 1] = ready[i];
                ready_n--;
                t += t_cs; //Context switch time
            }
            
        }
    }
    msg_event(t, "Simulator ended for FCFS");
    // TBA: Deallocate all the ready and waiting arrays so they're ready to use again for SRT?

    // TBA: Shortest Remaining Time (SRT)

    // TBA: Round Robin (RR)
    
    // TBA: output file stuff, don't forget to check if file can be opened (same process/error as w/ source file)
    
    // Problem: can't just free(array), dunno how to deallocate it
    exit(EXIT_SUCCESS);
}