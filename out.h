// Darien Keyack (661190088) and Corwin Aucoin (661178786)

float avg(int total, int count) {
    if (count == 0) return (float)total;
    else return ((float)total / (float)count);
}

void debug_float(float f, float v, FILE *output) {
    if (v - f > 0.01 || f - v > 0.01) fprintf(output, "** ERROR: correct value %.2f\n", f);
}

void debug_int(int i, int v, FILE *output) {
    if (i != v) fprintf(output, "** ERROR: correct value %i\n", i);
}

void out_params(char* algo, FILE *output, float burst, int wait_total, int wait_count, int turnaround_total, int turnaround_count, int switches, int preempts) {
    fprintf(output, "Algorithm %s\n", algo);
    fprintf(output, "-- average CPU burst time: %.2f ms\n", burst);
    #ifdef DEBUG
        int input = 1;
        float f = 403.85;
        debug_float(f, burst, output);
    #endif
    float wait = avg(wait_total, wait_count);
    fprintf(output, "-- average wait time: %.2f ms\n", wait);
    #ifdef DEBUG
        if (algo == "FCFS") f = 356.46;
        else if (algo == "SRT") f = 140.38;
        else f = 284.92;
        debug_float(f, wait, output);
    #endif
    float turnaround = avg(turnaround_total, turnaround_count);
    fprintf(output, "-- average turnaround time: %.2f ms\n", turnaround);
    #ifdef DEBUG
        if (algo == "FCFS") f = 766.31;
        if (algo == "SRT") f = 551.85;
        else f = 700.08;
        debug_float(f, turnaround, output);
    #endif
    fprintf(output, "-- total number of context switches: %i\n", switches);
    #ifdef DEBUG
        int i;
        if (algo == "FCFS") i = 13;
        if (algo == "SRT") i = 20;
        else i = 36;
        debug_int(i, switches, output);
    #endif
    fprintf(output, "-- total number of preemptions: %i\n", preempts);
    #ifdef DEBUG
        if (algo == "FCFS") i = 0;
        if (algo == "SRT") i = 7;
        else i = 23;
    #endif
}