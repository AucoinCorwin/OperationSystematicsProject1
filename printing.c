#include <stdio.h>
#include <stdlib.h>
#include "msg.h"

// Darien Keyack (661190088) and Corwin Aucoin (661178786)

int main(int argc, char *argv[]) {
    // Open true output file
    FILE *out = fopen(argv[1], "r");
    if (out == NULL) msg_error("Invalid true output file format");
    
    // Open generated output file
    FILE *gen = fopen(argv[2], "r");
    if (gen == NULL) msg_error("Invalid generated output file format");

    // Go through the outputs
    char line_g[100];
    char line_t[100];
    while (fgets(line_g, sizeof(line_g), gen) != NULL) {
        fgets(line_t, sizeof(line_t), out);
        if (line_g != line_t) {
            printf("Invalid match! %s and %s\n", line_g, line_t); 
            fflush(stdout);
        }
    }
    
    fclose(gen);
    fclose(out);
    exit(EXIT_SUCCESS);
}