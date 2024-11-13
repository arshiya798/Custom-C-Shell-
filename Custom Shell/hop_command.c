#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include "globals.h"

// Define ANSI color codes for red text and reset color
#define RED_COLOR "\033[31m"
#define RESET_COLOR "\033[0m"

extern char home_directory[4096];
static char current_directory[4096] = "";

void change_directory_and_print(const char *path) {
    // Store current directory in previous_directory before changing
    char temp_directory[4096];
    if (getcwd(temp_directory, sizeof(temp_directory)) == NULL) {
        fprintf(stderr, RED_COLOR "Error getting current directory" RESET_COLOR "\n");
        return;
    }

    // Change to the new directory
    if (chdir(path) == 0) {
        // Update current_directory to the new directory
        if (getcwd(current_directory, sizeof(current_directory)) != NULL) {
            printf("%s\n", current_directory);
            // Update previous_directory
            strncpy(previous_directory, temp_directory, sizeof(previous_directory) - 1);
            previous_directory[sizeof(previous_directory) - 1] = '\0';
        } else {
            fprintf(stderr, RED_COLOR "Error getting new directory" RESET_COLOR "\n");
        }
    } else {
        fprintf(stderr, RED_COLOR "Error changing directory" RESET_COLOR "\n");
    }
}

// Function to handle the hop command
void handle_hop(char **args) {
    int i = 1; // Start with the first argument (args[0] is "hop")

    // If no arguments are provided, hop to the home directory
    if (args[1] == NULL) {
        change_directory_and_print(home_directory);
        return;
    }

    while (args[i] != NULL) {
        if (strcmp(args[i], "~") == 0) {
            change_directory_and_print(home_directory);
        } else if (strcmp(args[i], "-") == 0) {
            if (strlen(previous_directory) > 0) {
                change_directory_and_print(previous_directory);
            } else {
                fprintf(stderr, RED_COLOR "hop: no previous directory" RESET_COLOR "\n");
            }
        } else if (strcmp(args[i], ".") == 0) {
            char directory[4096];
            // Stay in the current directory (no action needed)
            if (getcwd(directory, sizeof(directory)) == NULL) {
                fprintf(stderr, RED_COLOR "getcwd" RESET_COLOR "\n");
            }
            change_directory_and_print(directory);
        } else if (strcmp(args[i], "..") == 0) {
            change_directory_and_print("..");
        } else {
            char path[4096];

            // If the path starts with '~', replace it with the home directory
            if (args[i][0] == '~') {
                snprintf(path, sizeof(path), "%s%s", home_directory, args[i] + 1);
            } else {
                strncpy(path, args[i], sizeof(path));
            }

            change_directory_and_print(path);
        }

        i++;
    }
}
