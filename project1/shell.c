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
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "shell.h"
#include "path.h"
#include "utils.h"

int run(char *cmd, int in_parallel) {
    char *args[MAX_ARGS];
    char *token;
    int argc = 0;
    char *output_file = NULL;
    char *cmd_copy = strdup(cmd);
    char *original_cmd = cmd_copy;

    cmd_copy = strip(cmd_copy);
    
    if (cmd_copy[0] == '>') {
        print_error();
        free(original_cmd);
        return 0;
    }

    // Tokenize the command to handle arguments and redirection
    token = strtok(cmd_copy, " \t\n");
    while (token != NULL && argc < MAX_ARGS - 1) {
        if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " \t\n");
            if (token == NULL || strtok(NULL, " \t\n") != NULL) {
                print_error();
                free(original_cmd);
                return 0;
            }
            output_file = token;
            break;
        } else {
            args[argc++] = token;
        }
        token = strtok(NULL, " \t\n");
    }
    args[argc] = NULL;

    if (argc == 0) {
        free(original_cmd);
        return 0;
    }

    // Handle built-in commands
    if (strcmp(args[0], "exit") == 0) {
        if (argc > 1 || in_parallel) {
            print_error();
        } else {
            free(original_cmd);
            exit(0);
        }
        free(original_cmd);
        return 0;
    } else if (strcmp(args[0], "cd") == 0) {
        if (argc != 2 || chdir(args[1]) != 0 || in_parallel) {
            print_error();
        }
        free(original_cmd);
        return 0;
    } else if (strcmp(args[0], "path") == 0) {
        if (in_parallel) {
            print_error();
            free(original_cmd);
            return 0;
        }
        update_path(args, argc);
        free(original_cmd);
        return 0;
    }

    // Check if executable exists in the path
    char fullpath[MAX_INPUT];
    int found = 0;
    for (int i = 0; i < path_count; i++) {
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path[i], args[0]);
        if (access(fullpath, X_OK) == 0) {
            found = 1;
            break;
        }
    }
    if (!found) {
        print_error();
        free(original_cmd);
        return 0;
    }

    // Fork and execute the command
    pid_t pid = fork();
    if (pid == 0) {
        // In child process: handle redirection if needed
        if (output_file != NULL) {
            int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                print_error();
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        execv(fullpath, args);
        print_error(); // Exec failed
        exit(1);
    } else if (pid < 0) {
        print_error(); // Fork failed
    } else if (!in_parallel) {
        int status;
        waitpid(pid, &status, 0); // Parent waits for child to finish if not in parallel
    }

    free(original_cmd);
    return pid;
}

void prlcmds_handler(char *input) {
    char *commands[MAX_ARGS];
    int command_count = 0;
    char *saveptr;
    char *input_copy = strdup(input);

    // Split input into commands
    char *token = strtok_r(input_copy, "&", &saveptr);
    while (token != NULL && command_count < MAX_ARGS) {
        commands[command_count++] = strip(token);
        token = strtok_r(NULL, "&", &saveptr);
    }

    pid_t pids[MAX_ARGS];

    for (int i = 0; i < command_count; i++) {
        pids[i] = run(commands[i], 1);
    }

    // Parent waits for all parallel processes
    for (int i = 0; i < command_count; i++) {
        if (pids[i] > 0) {
            int status;
            waitpid(pids[i], &status, 0);
        }
    }

    free(input_copy);
}