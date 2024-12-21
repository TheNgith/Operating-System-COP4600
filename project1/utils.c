// Name: Thinh Nguyen

// NetID: ngt218

// Description: This program implements a RUSH shell 
// - a basic Unix shell that executes commands, handles 
// I/O redirection, and supports parallel process execution. 
// The shell provides essential built-in commands and external 
// program execution through a configurable search path, 
// demonstrating fundamental Unix shell functionality.

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "utils.h"

char error_message[30] = "An error has occurred\n";

void print_error(void) {
    write(STDERR_FILENO, error_message, strlen(error_message));
}

char* strip(char *str) {
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}