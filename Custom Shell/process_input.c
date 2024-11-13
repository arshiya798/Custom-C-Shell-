#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>
#include "process_input.h"
#include "hop_command.h"
#include "reveal_command.h"
#include "system_commands.h"
#include "log_command.h"
#include "proclore_command.h"
#include "seek_command.h"
#include <math.h>    // For floor()
#include <time.h>
#include "globals.h"
#include "pipe.h"
#include "activities.h"
#include "neonate.h"
#include "redirection.h"
#include "signal_handling.h"
#include "iMan.h"
#include "fgbg.h"

// Define ANSI color codes for red text and reset color
#define RED_COLOR "\033[31m"
#define RESET_COLOR "\033[0m"

#define MAX_COMMANDS 64  // Maximum number of commands that can be stored

void modify_input(const char *input, char *modified_input) {
    int mi_idx = 0;
    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == '&' && !check_ampersand_followed_by_pipe(input, i)) {
                modified_input[mi_idx++] = '&';
                modified_input[mi_idx++] = ';';
        } else {
            modified_input[mi_idx++] = input[i];
        }
    }
    modified_input[mi_idx] = '\0';
}

int check_ampersand_followed_by_pipe(const char *str, int ampersand_pos) {
    // Starting from the character right after the '&'
    for (int i = ampersand_pos + 1; str[i] != '\0'; i++) {
        if (!isspace(str[i])) {   // Skip spaces
            if (str[i] == '|') {
                return 1;         // True: First non-space character is '|'
            } else {
                return 0;         // False: Found a non-space character that isn't '|'
            }
        }
    }
    return 0; // If no non-space character was found after '&'
}

void tokenize_input_and_store_commands(char *modified_input, char *commands[], int *num_commands) {
    char *command = strtok(modified_input, ";");
    while (command != NULL && *num_commands < MAX_COMMANDS) {
        commands[*num_commands] = strdup(command);  // Store a copy of the command
        (*num_commands)++;
        command = strtok(NULL, ";");
    }
}

bool contains_pipe_symbols(const char *input) {
    // Check for pipe symbol
    return (strchr(input, '|') != NULL);
}

void process_input(char *input) {
    add_command_to_log(input);
    // Modify input to add ';' after every '&'
    char modified_input[2048];
    modify_input(input, modified_input);
    // printf("%s\n", modified_input);

    // Tokenize the input and Store all commands in a list
    char *commands[MAX_COMMANDS];
    int num_commands = 0;

    tokenize_input_and_store_commands(modified_input, commands, &num_commands);

    for (int i = 0; i < num_commands; i++) {
        if (commands[i] && strlen(commands[i]) > 0 && !is_whitespace_only(commands[i])) {
            process_command(commands[i]);
        }
        free(commands[i]);  // Free the memory allocated by strdup
    }
}

void process_command(const char *command) {
    // printf("cmg into process_command\n");
    int saved_stdin = dup(STDIN_FILENO);
    int saved_stdout = dup(STDOUT_FILENO);

    int is_background = 0;
    char present_command[4096];
    strcpy(present_command, command);

    trim_and_prepare_command_for_execution(present_command, &is_background);
    char trimmed_command[4096];
    strcpy(trimmed_command, present_command);

    if(strchr(command, '|') != NULL) {
        // printf("cmg here\n");
        handle_piping_command(present_command,is_background);
    }
    else{
        // printf("cmg\n");
        char *input_fd = NULL;
        char *output_fd = NULL;
        int append = 0;

        handle_redirections(present_command, &input_fd, &output_fd, &append);

        char **args = parse_command(present_command);

        if (!is_background) {
            // Measure the start time
            time_t start_time = time(NULL);

            // Execute the command
            execute_command(args, is_background, command);
            
            // Measure the end time
            time_t end_time = time(NULL);
            
            // Calculate the elapsed time
            double elapsed_time = difftime(end_time, start_time);
            int rounded_elapsed_time = (int)floor(elapsed_time); // Round down to nearest integer

            // If elapsed time is greater than 2 seconds, store it
            if (rounded_elapsed_time > 2) {
                char elapsed_info[1024];
                snprintf(elapsed_info, sizeof(elapsed_info), "%s: %ds", args[0], rounded_elapsed_time);
                add_to_prompt(elapsed_info);

                // For debugging: print the current prompt_extras after adding
                // printf("Updated prompt_extras: %s\n", prompt_extras);
            }
        } else {
            // Execute the command in the background
            execute_command(args, is_background, trimmed_command);
        }

        // Free the allocated memory for args
        free(args);

    }
    dup2(saved_stdin, STDIN_FILENO);
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdin);
    close(saved_stdout);
}

void trim_and_prepare_command_for_execution(char *command, int *is_background_ptr) {
    // Trim leading and trailing whitespace
    while (*command == ' ' || *command == '\t') command++;
    char *end = command + strlen(command) - 1;
    while (end > command && (*end == ' ' || *end == '\t' || *end == '\n')) end--;
    *(end + 1) = '\0';

    // Check if the command is meant to be run in the background
    if (*end == '&') {
        *is_background_ptr = 1;
        *end = '\0';
    }
    
    // Re-trim in case the background indicator (&) was removed
    end = command + strlen(command) - 1;
    while (end > command && (*end == ' ' || *end == '\t' || *end == '\n')) end--;
    *(end + 1) = '\0';
}

char** parse_command(char* command) {
    // Skip initial whitespace
    while (*command && isspace(*command)) {
        command++;
    }
    // Assuming a maximum of 64 arguments (including the command itself)
    char** args = (char**)malloc(64 * sizeof(char*));
    int i = 0;
    int in_quotes = 0;
    char *start = command;

    while (*command) {
        if (*command == '"') {
            in_quotes = !in_quotes;  // Toggle the in_quotes flag
        } else if (!in_quotes && isspace((unsigned char)*command)) {
            if (start != command) { // Only add non-empty arguments
                *command = '\0';  // Terminate the current argument
                command++;
                while (*command && isspace(*command)) {
                    command++;
                }
                command--;
                args[i++] = start;  // Store the argument
                start = command + 1;
            }
        }
        command++;
    }

    // Add the last argument
    if (*start) {
        args[i++] = start;
    }

    args[i] = NULL;  // Null-terminate the list of arguments
    return args;
}

int is_whitespace_only(const char *str) {
    while (*str) {
        if (!isspace((unsigned char)*str)) {
            return 0; // Not only whitespace
        }
        str++;
    }
    return 1; // Only whitespace
}

void execute_command(char **args, int is_background, const char* command) {
    // printf("cmg into execute command\n");
    if (strcmp(args[0], "hop") == 0) {
        handle_hop(args);
    }
    else if (strcmp(args[0], "reveal") == 0) {
        handle_reveal(args);
    } 
    else if (strcmp(args[0], "log") == 0) {
        // printf("cmg into log\n");
        handle_log_command(args);
    } 
    else if (strcmp(args[0], "proclore") == 0) {
        handle_proclore(args);
    }
    else if (strcmp(args[0], "seek") == 0) {
        handle_seek(args);
    }
    else if (strcmp(args[0], "activities") == 0) {
        display_background_processes();
    }
    else if (strcmp(args[0], "fg") == 0) {
        if (args[1] == NULL) {
            printf("Error: 'fg' command requires a process ID\n");
        } else {
            pid_t pid = (pid_t)atoi(args[1]);
            fg(pid);
        }
    }
    else if (strcmp(args[0], "bg") == 0) {
        if (args[1] == NULL) {
            printf("Error: 'bg' command requires a process ID\n");
        } else {
            pid_t pid = (pid_t)atoi(args[1]);
            bg(pid);
        }
    }
    else if (strcmp(args[0], "ping") == 0) {
        if (args[1] == NULL || args[2] == NULL) {
            printf("Error: 'ping' command requires a process ID and a signal number\n");
        } else {
            pid_t pid = (pid_t)atoi(args[1]);
            int signal_number = atoi(args[2]);
            ping_process(pid, signal_number);
        }
    }
    else if (strcmp(args[0], "neonate") == 0) {
        if (args[1] != NULL && strcmp(args[1], "-n") == 0) {
            if (args[2] != NULL) {
                int interval = atoi(args[2]);  // Convert interval argument to integer
                if (interval > 0) {
                    neonate_functionality(interval);  // Call neonate with the interval
                } else {
                    printf("Error: Invalid interval. Please provide a valid positive integer.\n");
                }
            } else {
                printf("Error: 'neonate' command requires an interval argument after '-n'.\n");
            }
        } else {
            printf("Error: 'neonate' command requires '-n' followed by an interval argument.\n");
        }
    }

    else if (strcmp(args[0], "iMan") == 0) {
        if (args[1] != NULL) {  // Check if a command name was provided
            ManPage man_page;
            fetch_man_page(args[1], &man_page);
            printf("%s\n", args[1]);  // Print the command name
            printf("%.*s", (int)man_page.length, man_page.content);  // Print the fetched man page content
        } else {
            printf("Usage: iMan <command_name>\n");
        }
    }
    else {
        // printf("cmg into execute_system_command\n");
        execute_system_command(args, is_background, command);
    }
}


void add_to_prompt(const char *extra) {
    if (strlen(prompt_extras) > 0) {
        strncat(prompt_extras, "; ", sizeof(prompt_extras) - strlen(prompt_extras) - 1);
    }
    strncat(prompt_extras, extra, sizeof(prompt_extras) - strlen(prompt_extras) - 1);
}
