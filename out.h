// Darien Keyack (661190088) and Corwin Aucoin (661178786)

float avg(int total, int count) {
    if (count == 0) return (float)total;
    else return ((float)total / (float)count);
}

void debug_float(float f, float v, FILE *output) {
    if (v - f > 0.01 || f - v > 0.01) fprintf(output, "** ERROR: correct value %.2f\n", f);
}

void debug_int(int i, int v, FILE *output) {
    if (i != v) fprintf(output, "** ERROR: correct value %i **\n", i);
}

void out_params(char* input, char* algo, FILE *output, float burst, int wait_total, int wait_count, int turnaround_total, int turnaround_count, int switches, int preempts) {
    fprintf(output, "Algorithm %s\n", algo);
    fprintf(output, "-- average CPU burst time: %.2f ms\n", burst);
    #ifdef DEBUG
        debug_float(403.85, burst, output);
    #endif
    float wait = avg(wait_total, wait_count);
    fprintf(output, "-- average wait time: %.2f ms\n", wait);
    #ifdef DEBUG
        if (algo == "FCFS") debug_float(356.46, wait, output);
        else if (algo == "SRT") debug_float(140.38, wait, output);
        else debug_float(284.92, wait, output);
    #endif
    float turnaround = avg(turnaround_total, turnaround_count);
    fprintf(output, "-- average turnaround time: %.2f ms\n", turnaround);
    #ifdef DEBUG
        if (algo == "FCFS") debug_float(766.31, turnaround, output);
        else if (algo == "SRT") debug_float(551.85, turnaround, output);
        else debug_float(700.08, turnaround, output);
    #endif
    fprintf(output, "-- total number of context switches: %i\n", switches);
    #ifdef DEBUG
        int i;
        if (algo == "FCFS") debug_int(13, switches, output);
        else if (algo == "SRT") debug_int(20, switches, output);
        else debug_int(36, switches, output);
    #endif
    fprintf(output, "-- total number of preemptions: %i\n", preempts);
    #ifdef DEBUG
        if (algo == "FCFS") debug_int(0, preempts, output);
        else if (algo == "SRT") debug_int(7, preempts, output);
        else debug_int(23, preempts, output);
    #endif
}