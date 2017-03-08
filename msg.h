#include "process.h"

void print_time(int t) {
    printf("time %ims: ", t);
}

void msg_error(char *msg) {
    fprintf(stderr, "ERROR: %s\n", msg);
    exit(EXIT_FAILURE);
}

void msg_queue(struct Process *ready, int n) {
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

void msg_space() {
    printf("\n");
    fflush(stdout);
}

void msg_event(int t, char *msg) {
    print_time(t);
    printf("%s\n", msg);
    fflush(stdout);
}

void msg_event_q_i(int t, char id, char *msg, char *msg2, int i, struct Process *ready, int n) {
    print_time(t);
    if (id != ' ') printf("Process %c ", id); // Assume a process can't have an id of ' '
    printf("%s ", msg);
    if (strcmp(msg2, "") != 0) printf("%i%s ", i, msg2);
    msg_queue(ready, n);
}

void msg_event_q(int t, char id, char *msg, struct Process *ready, int n) {
    msg_event_q_i(t, id, msg, "", 0, ready, n);
}

void msg_io_preempt(int t, char id1, char id2, struct Process *ready, int n) {
    printf("time %ims: Process %c completed I/O and will preempt %c ", t, id1, id2);
    msg_queue(ready, n);
}