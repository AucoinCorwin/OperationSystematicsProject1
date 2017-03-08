#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "msg.h"

void reset(int *t, int *ready_n, int *waiting_n, int n, struct Process **waiting, bool *running_active, int *blocked_n) {
    msg_space();
    *t = 0;
    *ready_n = 0;
    *waiting_n = n;
    int i;
    running_active = false;
    blocked_n = 0;
}
 /*void srt_add(struct Process *array, struct Process proc, int *arrsize){
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
    }*/

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
    struct Process *ready = (struct Process*) calloc(n, sizeof(struct Process));
    int ready_n = 0;
    struct Process *waiting = (struct Process*) calloc(n, sizeof(struct Process));
    int waiting_n = n;
    for (i = 0; i < n; i++) waiting[i] = array[i];
    struct Process running;
    bool running_active = false;
    struct Process *blocked = (struct Process*) calloc(n, sizeof(struct Process));
    int blocked_n = 0;
    bool increment;
    msg_sim_start(t, "FCFS", ready, ready_n);
    
    while (ready_n > 0 || waiting_n > 0 || blocked_n > 0 || running_active) {
        increment = true;
        // Check for new arrivals
        for (i = 0; i < waiting_n; ++i) {
            if (waiting[i].arrive <= t) {
                ready_n++;
                ready[ready_n - 1] = waiting[i];
                msg_added_ready(t, ready[ready_n - 1].id, "arrived and", ready, ready_n);
                waiting_n--;
                for (j = i; j < waiting_n; j++) waiting[j] = waiting[j + 1];
                increment = false;
            }
        }
        // Check for finished I/O
        for (i = 0; i < blocked_n; ++i) {
            if (blocked[i].arrive <= t) {
                ready_n++;
                ready[ready_n - 1] = blocked[i];
                msg_added_ready(t, ready[ready_n - 1].id, "completed I/O;", ready, ready_n);
                blocked_n--;
                for (j = i; j < blocked_n; j++) blocked[j] = blocked[j + 1];
                increment = false;
            }
        }
        // Set running, if possible/none already
        if (increment && !running_active && ready_n > 0) {   //
            running = ready[0];
            running_active = true;
            ready_n--;
            for (i = 0; i < ready_n; i++) ready[i] = ready[i + 1];
            t += t_cs/2;
            msg_event_q(t, running.id, "started using the CPU", ready, ready_n);
            running.arrive = t + running.burst_time;
            increment = false;
        }
        // Update time if nothing else has been done this tick
        if (increment) {
            t++;
            // Update running, if possible
            if (running_active && running.arrive <= t) {
                running.burst_num--;
                running_active = false;
                // Add to blocked, if possible
                if (running.burst_num > 0) {
                    msg_event_q_i(t, running.id, "completed a CPU burst;", " bursts to go", running.burst_num, ready, ready_n);
                    t += t_cs/2;
                    running.arrive = t + running.io;
                    msg_event_q_i(t, running.id, "switching out of CPU; will block on I/O until time", "ms", running.arrive, ready, ready_n);
                    blocked_n++;
                    blocked[blocked_n - 1] = running;
                }
                // Terminate if finished
                else {
                    msg_event_q(t, running.id, "terminated", ready, ready_n);
                    t += t_cs/2;
                }
            }
        }
    }
    msg_sim_end(t, "FCFS");

    // Shortest Remaining Time (SRT)
    reset(&t, &ready_n, &waiting_n, n, &waiting, &running_active, &blocked_n);
    for (i = 0; i < n; i++) {
        waiting[i] = array[i];
        waiting[i].burst_left = waiting[i].burst_time;
    }
    msg_sim_start(t, "SRT", ready, ready_n);
    while (ready_n > 0 || waiting_n > 0 || blocked_n > 0 || running_active) {
        increment = true;
        // Check for new arrivals
        for (i = 0; i < waiting_n; ++i) {
            if (waiting[i].arrive <= t) {
                // Preemptive
                if (running_active && ready[ready_n - 1].burst_left < running.burst_left) {
                    msg_preempt(t, waiting[i].id, running.id, "arrived", ready, ready_n);
                    ready_n++;
                    ready[ready_n - 1] = running;
                    t += t_cs;
                    running = waiting[i];
                    msg_event_q(t, running.id, "started using the CPU", ready, ready_n);
                }
                // Non-preemptive
                else {
                    ready_n++;
                    ready[ready_n - 1] = waiting[i];
                    msg_added_ready(t, ready[ready_n - 1].id, "arrived and", ready, ready_n);
                }
                waiting_n--;
                for (j = i; j < waiting_n; j++) waiting[j] = waiting[j + 1];
                increment = false;
            }
        }
        for (i = 0; i < blocked_n; ++i) {
            if (blocked[i].arrive <= t) {
                if (blocked[i].burst_time < running.burst_left) {
                    msg_preempt(t, blocked[i].id, running.id, "completed I/O", ready, ready_n);
                    ready_n++;
                    ready[ready_n - 1] = running;
                    running = blocked[i];
                    running.burst_left = running.burst_time;
                    
                    t += t_cs;
                    msg_event_q(t, running.id, "started using the CPU", ready, ready_n);
                    blocked_n--;
                    for(j = i; j < blocked_n; j++) blocked[j] = blocked[j + 1];
                    increment = false;
                }
                else{
                    ready_n++;
                    ready[ready_n - 1] = blocked[i];
                    ready[ready_n - 1].burst_left = ready[ready_n - 1].burst_time;
                    msg_event_q(t, ready[ready_n - 1].id, "completed I/O; added to ready queue", ready, ready_n);
                    blocked_n--;
                    for (j = i; j < blocked_n; j++) blocked[j] = blocked[j + 1];
                    increment = false;
                }
                
            }
        }
        // Set running, if possible/none already
        if (increment && !running_active && ready_n > 0) {
            j = 0;
            for (i = 0; i < ready_n; i++) if (ready[i].burst_left < ready[j].burst_left) j = i;
            running = ready[j];
            running_active = true;
            ready_n--;
            for (i = j; i < ready_n; i++) ready[i] = ready[i + 1];
            t += t_cs/2;
            msg_event_q(t, running.id, "started using the CPU", ready, ready_n);
            
            running.arrive = t + running.burst_time;
            increment = false;
        }
        // Update time if nothing else has been done this tick
        if (increment) {
            
            // Update running, if possible
            if (running_active) {
                if (running.burst_left <= 0) {
                    running.burst_left = 0;
                    running.burst_num--;
                    running_active = false;
                    
                    // Add to blocked, if possible
                    if (running.burst_num > 0) {
                        msg_event_q_i(t, running.id, "completed a CPU burst;", " bursts to go", running.burst_num, ready, ready_n);
                        msg_event_q_i(t, running.id, "switching out of CPU; will block on I/O until time", "ms", (t + (t_cs/2) +  running.io), ready, ready_n);
                        t += t_cs/2;
                        running.arrive = t + running.io;
                        blocked_n++;
                        blocked[blocked_n - 1] = running;
                        t--;
                    }
                    // Terminate if finished
                    else {
                        msg_event_q(t, running.id, "terminated", ready, ready_n);
                        t += t_cs/2;
                    }
                }
                else running.burst_left--;
            }
            t++;
        }
    }
    t--;
    msg_sim_end(t, "SRT");

    // Round Robin (RR)
    reset(&t, &ready_n, &waiting_n, n, &waiting, &running_active, &blocked_n);
    for (i = 0; i < n; i++) {
        waiting[i] = array[i];
        waiting[i].burst_left = waiting[i].burst_time;
    }
    msg_sim_start(t, "RR", ready, ready_n);
     while (ready_n > 0 || waiting_n > 0 || blocked_n > 0 || running_active) {
        increment = true;
        // Check for new arrivals
        for (i = 0; i < waiting_n; ++i) {
            if (waiting[i].arrive <= t) {
                ready_n++;
                ready[ready_n - 1] = waiting[i];
                msg_added_ready(t, ready[ready_n - 1].id, "arrived and", ready, ready_n);
                waiting_n--;
                for (j = i; j < waiting_n; j++) waiting[j] = waiting[j + 1];
                increment = false;
            }
        }
        // Check for finished I/O
        for (i = 0; i < blocked_n; ++i) {
            if (blocked[i].arrive <= t) {
                blocked[i].burst_left = blocked[i].burst_time;
                ready_n++;
                ready[ready_n - 1] = blocked[i];
                msg_added_ready(t, ready[ready_n - 1].id, "completed I/O;", ready, ready_n);
                blocked_n--;
                for (j = i; j < blocked_n; j++) blocked[j] = blocked[j + 1];
                increment = false;
            }
        }
        // Set running, if possible/none already
        if (increment && !running_active && ready_n > 0) {   //
            running = ready[0];
            running_active = true;
            ready_n--;
            for (i = 0; i < ready_n; i++) ready[i] = ready[i + 1];
            t += t_cs/2;
            msg_event_q(t, running.id, "started using the CPU", ready, ready_n);
            running.arrive = t + t_slice;
            increment = false;

        }
        // Update time if nothing else has been done this tick
        if (increment) {
            t++;
            running.burst_left--;
            // Update running, if possible
            
            if (running_active && running.burst_left <= 0) {
                running.burst_num--;
                running_active = false;
                // Add to blocked, if possible
                if (running.burst_num > 0) {
                    
                    msg_event_q_i(t, running.id, "completed a CPU burst;", " bursts to go", running.burst_num, ready, ready_n);
                    
                    msg_event_q_i(t, running.id, "switching out of CPU; will block on I/O until time", "ms", (t + (t_cs/2) + running.io), ready, ready_n);
                    t += t_cs/2;
                    running.arrive = t + running.io;
                    blocked_n++;
                    blocked[blocked_n - 1] = running;
                }
                // Terminate if finished
                else {
                    msg_event_q(t, running.id, "terminated", ready, ready_n);
                    t += t_cs/2;
                }
            }
            else if (running_active && running.arrive <= t){
                //Then the time slice is up, and it needs to be added to the back of the ready queue
                if(ready_n > 0) {
                    msg_slice_preempt(t, running.id, running.burst_left, ready, ready_n);
                    ready_n++;
                    ready[ready_n - 1] = running;
                    running_active = false;
                    t += t_cs/2;
                }
                else {
                    running.arrive += t + t_slice;
                    msg_event_q(t, ' ', "Time slice expired; no preemption because ready queue is empty", ready, ready_n);
                }
            }
        }
    }
    msg_sim_end(t, "RR");
    
    // TBA: output file stuff, don't forget to check if file can be opened (same process/error as w/ source file)
    // Problem: can't just free(array), dunno how to deallocate it
    exit(EXIT_SUCCESS);
}