#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "log_command.h"
#include "process_input.h"
#include "globals.h"

// Define ANSI color codes for red text and reset color
#define RED_COLOR "\033[31m"
#define RESET_COLOR "\033[0m"

void add_command_to_log(const char *command) {
    // Skip commands that start with "log"
    if (strncmp(command, "log", 3) == 0 && (command[3] == '\0' || isspace(command[3]))) {
        return;
    }

    // Do not add if the command is the same as the last command
    if (log_count > 0 && strcmp(command_log[(next_index - 1 + 15) % 15], command) == 0) {
        return;
    }

    // Store the command in the circular log array
    strncpy(command_log[next_index], command, sizeof(command_log[next_index]) - 1);
    command_log[next_index][sizeof(command_log[next_index]) - 1] = '\0'; // Ensure null-termination

    next_index = (next_index + 1) % 15;
    // printf("next index is %d\n", next_index);

    if (log_count < 15) {
        log_count++;
    }

    // Clear the log file and write the updated log
    FILE *log_file = fopen(log_file_path, "w");
    if (log_file != NULL) {
        int start_index = (next_index - log_count + 15) % 15;
        for (int i = 0; i < log_count; i++) {
            int current_index = (start_index + i) % 15;
            size_t len = strlen(command_log[current_index]);

            // Remove any trailing newline
            if (len > 0 && command_log[current_index][len - 1] == '\n') {
                command_log[current_index][len - 1] = '\0';
            }

            fprintf(log_file, "%s\n", command_log[current_index]);
        }
        fclose(log_file);
    }
}

// Function to display the command log
void display_log() {
    int start_index = (next_index - log_count + 15) % 15;
    for (int i = 0; i < log_count; i++) {
        int index = (start_index + i) % 15;
        printf("%s\n", command_log[index]);
    }
}

// Function to purge the log both in memory and on disk
void purge_log() {
    // Clear the in-memory log
    log_count = 0;
    next_index = 0;

    // Clear the log file
    FILE *log_file = fopen(log_file_path, "w");
    if (log_file != NULL) {
        fclose(log_file);
    }
}

// Function to execute a command from the log by index
void execute_command_from_log(int index) {
    if (index < 1 || index > log_count) {
        printf(RED_COLOR "Invalid log index: %d" RESET_COLOR "\n", index);
        return;
    }

    int actual_index = (next_index - index + 15) % 15;
    printf("Executing: %s\n", command_log[actual_index]);

    // You can choose to not log this command execution as per the specification
    process_input(command_log[actual_index]);
}

// Function to handle the 'log' command with arguments
void handle_log_command(char **args) {
    if (args[1] == NULL) {
        display_log();
    } else if (strcmp(args[1], "purge") == 0) {
        purge_log();
    } else if (strcmp(args[1], "execute") == 0 && args[2] != NULL) {
        int index = atoi(args[2]);
        execute_command_from_log(index);
    } else {
        printf(RED_COLOR "Unknown log command or arguments." RESET_COLOR "\n");
    }
}
