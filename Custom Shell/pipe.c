#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>
#include "pipe.h"
#include "process_input.h"

#define MAX_ARGS 100
#define MAX_SUBCOMMANDS 20

// Red color escape codes
#define RED_TEXT "\033[31m"
#define RESET_TEXT "\033[0m"

// Function to parse piped commands
void parse_piping_command(const char* command, char* subcommands[], int is_background) {
    char given_command[5000];
    strcpy(given_command, command);

    // Check for invalid pipe usage at the start or end of the command
    if (given_command[strlen(command) - 1] == '|' || given_command[0] == '|') {
        // Print error message in red
        fprintf(stderr, RED_TEXT "Invalid pipe command" RESET_TEXT "\n");
        exit(EXIT_FAILURE);
    }

    int i = 0;
    char* token = strtok(given_command, "|"); 
    while (token != NULL && i < MAX_SUBCOMMANDS) {
        subcommands[i++] = token;     
        token = strtok(NULL, "|");      
    }
    subcommands[i] = NULL;
}

// Function to check if a string contains only whitespace
int is_whitespaceonly(const char* str) {
    while (*str) {
        if (!isspace((unsigned char)*str)) {
            return 0;  
        }
        str++;
    }
    return 1; 
}

// Function to handle and execute piped commands
void handle_piping_command(const char *command, int is_background) {
    char given_command[5000];
    strcpy(given_command, command);
    char *subcommands[MAX_SUBCOMMANDS] = {NULL};  
    parse_piping_command(given_command, subcommands, is_background);

    int num_subcommands = 0;
    while (subcommands[num_subcommands] != NULL && num_subcommands < MAX_SUBCOMMANDS) {
        num_subcommands++;
    }

    execute_piped_commands(subcommands, num_subcommands, is_background);
}

// Function to execute piped commands
void execute_piped_commands(char* subcommands[], int num_commands, int is_background) {

    int pipefd[2 * (num_commands - 1)];  

    // Check for invalid use of '&' in piped commands
    for (int i = 0; i < num_commands - 1; i++) {
        if (strchr(subcommands[i], '&') != NULL) {
            // Print error message in red
            fprintf(stderr, RED_TEXT "Invalid pipe command: '&' found in a piped command" RESET_TEXT "\n");
            exit(EXIT_FAILURE);
        }
    }

    // Check for empty or whitespace-only subcommands
    for (int i = 0; i < num_commands; i++) {
        if (is_whitespaceonly(subcommands[i]) || strlen(subcommands[i]) == 0) {
            // Print error message in red
            fprintf(stderr, RED_TEXT "Invalid pipe command: empty or whitespace-only subcommand" RESET_TEXT "\n");
            exit(EXIT_FAILURE);
        }
    }

    // Create pipes
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipefd + i * 2) == -1) {
            // Print error message in red
            fprintf(stderr, RED_TEXT "Pipe creation failed" RESET_TEXT "\n");
            perror("pipe failed");
            exit(EXIT_FAILURE);
        }
    }

    // Fork and execute commands
    for (int i = 0; i < num_commands; i++) {
        pid_t pid = fork();
        if (pid == 0) {  // Child process

            // Set up input from previous command's pipe
            if (i > 0) {
                dup2(pipefd[(i - 1) * 2], STDIN_FILENO); 
            }

            // Set up output to the next command's pipe
            if (i < num_commands - 1) {
                dup2(pipefd[i * 2 + 1], STDOUT_FILENO);  
            }

            // Close all pipe file descriptors
            for (int j = 0; j < 2 * (num_commands - 1); j++) {
                close(pipefd[j]);
            }

            // If it's the last command and is_background is true, add '&'
            if (i == num_commands - 1 && is_background) {
                int len = strlen(subcommands[i]);
                subcommands[i][len] = '&';
                subcommands[i][len + 1] = '\0';
            }

            process_command(subcommands[i]);  // Execute the command
            exit(EXIT_SUCCESS); 
        } 
        else if (pid < 0) {  // Error during fork
            // Print error message in red
            fprintf(stderr, RED_TEXT "Fork failed" RESET_TEXT "\n");
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
    }

    // Close all pipe file descriptors in the parent process
    for (int j = 0; j < 2 * (num_commands - 1); j++) {
        close(pipefd[j]);
    }

    // Wait for all child processes to finish
    for (int i = 0; i < num_commands; i++) {
        wait(NULL);
    }
}
