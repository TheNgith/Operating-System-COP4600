// Name: Thinh Nguyen

// NetID: ngt218

// Description: This program implements a RUSH shell 
// - a basic Unix shell that executes commands, handles 
// I/O redirection, and supports parallel process execution. 
// The shell provides essential built-in commands and external 
// program execution through a configurable search path, 
// demonstrating fundamental Unix shell functionality.

#ifndef PATH_H
#define PATH_H

extern char *path[];
extern int path_count;

void initialize_path(void);
void cleanup_path(void);
void update_path(char **args, int argc);

#endif