// Darien Keyack (661190088) and Corwin Aucoin (661178786)

struct Process {
    char id;
    int arrive;
    int burst_time;
    int burst_num;
    int burst_left;
    int io;
    int arrive_wait;
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