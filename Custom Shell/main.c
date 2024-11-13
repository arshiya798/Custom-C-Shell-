#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "globals.h"
#include "process_input.h"
#include "system_commands.h"
#include "seek_command.h"
#include "signal_handling.h"
#include "iMan.h"
#include "fgbg.h"


#define COLOR_RED "\033[0;31m"
#define COLOR_BLUE "\033[0;34m"
#define COLOR_RESET "\033[0m"

char username[256];
char systemname[256];
char home_directory[4096];
char previous_directory[4096] = ""; 
char command_log[15][1024];
char log_file_path[5100];
char prompt_extras[4096] = "";
char foreground_command[4096] = "";
pid_t foreground_pid = -1;
int log_count;
int next_index;

// Define global variables
BackgroundProcess bg_processes[MAX_BG_PROCESSES];
int bg_count = 0;

int count_commands_in_log_file() {
    FILE *log_file = fopen(log_file_path, "r");
    int count = 0;

    if (log_file != NULL) {
        char line[1024];
        while (fgets(line, sizeof(line), log_file)) {
            count++;
        }
        fclose(log_file);
    } else {
        // printf(COLOR_RED "Log file not found.\n" COLOR_RESET);
    }
    return count;
}

void load_log_from_file() {
    FILE *log_file = fopen(log_file_path, "r");
    if (log_file != NULL) {
        char line[1024];
        // Reset log state
        log_count = 0;
        next_index = 0;
        
        while (fgets(line, sizeof(line), log_file)) {
            // Remove the newline character from the line if present
            line[strcspn(line, "\n")] = 0;

            // Skip empty lines
            if (strlen(line) == 0) {
                continue;
            }

            // Store the command in the log array
            strncpy(command_log[next_index], line, sizeof(command_log[next_index]) - 1);
            command_log[next_index][sizeof(command_log[next_index]) - 1] = '\0'; // Null-terminate

            next_index = (next_index + 1) % 15;
            if (log_count < 15) {
                log_count++;
            }
        }
        fclose(log_file);
    } else {
        // printf(COLOR_RED "No existing log file found.\n" COLOR_RESET);
    }
}

void display_prompt(const char *username, const char *systemname, const char *current_directory, const char *home_directory) {
    if (strncmp(current_directory, home_directory, strlen(home_directory)) == 0) {
        // When current directory is the home directory
        if (strlen(prompt_extras) > 0) {
            printf(COLOR_BLUE "<%s@%s:~%s %s> " COLOR_RESET, username, systemname, current_directory + strlen(home_directory), prompt_extras);
        } else {
            printf(COLOR_BLUE "<%s@%s:~%s> " COLOR_RESET, username, systemname, current_directory + strlen(home_directory));
        }
    } else {
        // When current directory is not the home directory
        if (strlen(prompt_extras) > 0) {
            printf(COLOR_BLUE "<%s@%s:%s %s> " COLOR_RESET, username, systemname, current_directory, prompt_extras);
        } else {
            printf(COLOR_BLUE "<%s@%s:%s> " COLOR_RESET, username, systemname, current_directory);
        }
    }
}

int main() {
    char current_directory[4096];
    signal(SIGINT, handle_sigint);
    signal(SIGTSTP, handle_sigtstp); 
    load_myshrc("myshrc.txt");
    // Get the username
    if (getlogin_r(username, sizeof(username)) != 0) {
        printf(COLOR_RED "getlogin_r failed\n" COLOR_RESET);
        return 1;
    }

    // Get the system name
    if (gethostname(systemname, sizeof(systemname)) != 0) {
        printf(COLOR_RED "gethostname failed\n" COLOR_RESET);
        return 1;
    }

    // Get the home directory (initial directory when the shell starts)
    if (getcwd(home_directory, sizeof(home_directory)) == NULL) {
        printf(COLOR_RED "getcwd failed\n" COLOR_RESET);
        return 1;
    }

    snprintf(log_file_path, sizeof(log_file_path), "%s/command_log.txt", home_directory);
    log_count = count_commands_in_log_file();
    next_index = log_count;
    load_log_from_file();

    while (1) {
        
        // printf("cmg into main\n");
        check_background_processes();
        
        // Get the current directory
        if (getcwd(current_directory, sizeof(current_directory)) == NULL) {
            printf(COLOR_RED "getcwd failed\n" COLOR_RESET);
            return 1;
        }

        // Display the prompt
        display_prompt(username, systemname, current_directory, home_directory);

        // Read the user input
        char input[1024];
        if(fgets(input, sizeof(input), stdin) == NULL){
            if(feof(stdin)){
                handle_eof();
            }
        }

        process_input(input);
    }

    return 0;
}
