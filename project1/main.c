// Name: Thinh Nguyen

// NetID: ngt218

// Description: This program implements a RUSH shell 
// - a basic Unix shell that executes commands, handles 
// I/O redirection, and supports parallel process execution. 
// The shell provides essential built-in commands and external 
// program execution through a configurable search path, 
// demonstrating fundamental Unix shell functionality.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"
#include "path.h"
#include "utils.h"

int main(int argc, char *argv[]) {
    if (argc != 1) {
        print_error();
        exit(1);
    }

    initialize_path();

    while (1) {
        printf("rush> ");
        fflush(stdout);

        char input[MAX_INPUT];
        if (fgets(input, MAX_INPUT, stdin) == NULL) {
            break; // Exit on EOF
        }

        // Remove trailing newline
        input[strcspn(input, "\n")] = 0;

        // Handle empty lines
        if (strlen(strip(input)) == 0) {
            continue;
        }

        // Check for parallel commands
        if (strchr(input, '&') != NULL) {
            prlcmds_handler(input);
        } else {
            run(input, 0);
        }
    }

    cleanup_path();
    return 0;
}