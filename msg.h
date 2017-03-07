#include "process.h"

void msg_error(char *msg) {
    fprintf(stderr, "ERROR: %s\n", msg);
    exit(EXIT_FAILURE);
}

void msg_space() {
    printf("\n");
    fflush(stdout);
}

void msg_event(int t, char *msg) {
    printf("time %ims: %s\n", t, msg);
    fflush(stdout);
}

void msg_event_q_i(int t, char id, char *msg, char *msg2, int i, struct Process *ready, int n) {
    printf("time %ims: ", t);
    if (id != ' ') printf("Process %c ", id); // Assume a process can't have an id of ' '
    printf("%s ", msg);
    if (strcmp(msg2, "") != 0) printf("%i%s ", i, msg2);
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