// Name: Thinh Nguyen

// NetID: ngt218

// Description: This program implements a RUSH shell 
// - a basic Unix shell that executes commands, handles 
// I/O redirection, and supports parallel process execution. 
// The shell provides essential built-in commands and external 
// program execution through a configurable search path, 
// demonstrating fundamental Unix shell functionality.

#include <stdlib.h>
#include <string.h>
#include "path.h"
#include "shell.h"

char *path[MAX_ARGS];
int path_count = 1;

void initialize_path(void) {
    // Initialize path to contain /bin initially
    path[0] = strdup("/bin");
    path[1] = NULL;
}

void cleanup_path(void) {
    // Free allocated memory for path
    for (int i = 0; i < path_count; i++) {
        free(path[i]);
    }
}

void update_path(char **args, int argc) {
    for (int i = 0; i < path_count; i++) {
        free(path[i]);
    }
    path_count = 0;
    for (int i = 1; i < argc; i++) {
        path[path_count++] = strdup(args[i]);
    }
    path[path_count] = NULL;
}