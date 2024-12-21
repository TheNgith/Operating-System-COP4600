// Name: Thinh Nguyen

// NetID: ngt218

// Description: This program implements a RUSH shell 
// - a basic Unix shell that executes commands, handles 
// I/O redirection, and supports parallel process execution. 
// The shell provides essential built-in commands and external 
// program execution through a configurable search path, 
// demonstrating fundamental Unix shell functionality.

#ifndef SHELL_H
#define SHELL_H

#define MAX_INPUT 255
#define MAX_ARGS 64

void print_error(void);
char* strip(char *str);
int run(char *cmd, int in_parallel);
void prlcmds_handler(char *input);

#endif