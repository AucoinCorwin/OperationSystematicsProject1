void out_start(FILE *output, char *algo) {
    fprintf(output, "Algorithm %s\n", algo);
}

float avg(int total, int count) {
    if (count == 0) return (float)total;
    else return ((float)total / (float)count);
}

void out_params(FILE *output, float burst, int wait_total, int wait_count, int turnaround_total, int turnaround_count, int switches, int preempts) {
    fprintf(output, "-- average CPU burst time: %.2f ms\n", burst);
    fprintf(output, "-- average wait time: %.2f ms\n", avg(wait_total, wait_count));
    fprintf(output, "-- average turnaround time: %.2f ms\n", avg(turnaround_total, turnaround_count));
    fprintf(output, "-- total number of context switches: %i\n", switches);
    fprintf(output, "-- total number of preemptions: %i\n", preempts);
}