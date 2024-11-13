#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "system_commands.h"
#include "globals.h"

#define COLOR_RED "\033[0;31m"
#define COLOR_RESET "\033[0m"
void execute_system_command(char **args, int is_background, const char* command) {
    pid_t pid = fork();

    if (pid < 0) {
        printf(COLOR_RED "Fork failed\n" COLOR_RESET);
        return;
    }

    if (pid == 0) {  // Child process
        if (execvp(args[0], args) == -1) {
            // Print error message in the child process
            printf(COLOR_RED "Execution failed\n" COLOR_RESET);
            exit(EXIT_FAILURE);  // Ensure the child exits immediately after the error
        }
    } else {  // Parent process
        if (!is_background) {  // Foreground process
            foreground_pid = pid;  // Set the global foreground_pid
            strncpy(foreground_command, command, 4095);  // Update foreground_command
            foreground_command[4095] = '\0';  // Ensure null termination
            int status;
            waitpid(pid, &status, WUNTRACED);  // Wait for child, including stopped by signal
            if (WIFSTOPPED(status)) {
                printf("Child stopped by signal %d\n", WSTOPSIG(status));
            } else if (WIFEXITED(status)) {
                printf("Child terminated normally with exit status %d\n", WEXITSTATUS(status));
            }
        } else {  // Background process
            // Background process: Print PID and add process to tracking
            printf("%s : [%d] \n", command, pid);
            add_command_to_bg_list(command, pid);

            // Check if the process should be stopped (interactive)
            if (strcmp(args[0], "vi") == 0 || strcmp(args[0], "vim") == 0) {
                kill(pid, SIGSTOP);  // Stop the process
                update_bg_process_state(pid, "Stopped");  // Update the state
                printf("[%d] Stopped\n", pid);
            } else {
                update_bg_process_state(pid, "Running");  // Default state for other processes
            }
        }
    }
}

void check_background_processes() {
    int status;
    pid_t pid;

    for (int i = 0; i < bg_count; i++) {
        pid = waitpid(bg_processes[i].pid, &status, WNOHANG);
        if (pid > 0) {
            if (WIFEXITED(status)) {
                if (WEXITSTATUS(status) == 0) {
                    // Normal exit
                    update_bg_process_state(pid, "Terminated");
                    printf("%s exited normally\n", bg_processes[i].command);
                }
            } else if (WIFSIGNALED(status)) {
                update_bg_process_state(pid, "Terminated");
                printf("%s killed by signal\n", bg_processes[i].command);
            }

            // Remove the process from the list
            for (int j = i; j < bg_count - 1; j++) {
                bg_processes[j] = bg_processes[j + 1];
            }
            bg_count--;
            i--;  // Adjust index to account for removed process
        }
    }
}

void add_command_to_bg_list(const char *command, pid_t pid) {
    if (bg_count < MAX_BG_PROCESSES) {
        bg_processes[bg_count].pid = pid;
        strncpy(bg_processes[bg_count].command, command, sizeof(bg_processes[bg_count].command) - 1);
        bg_processes[bg_count].command[sizeof(bg_processes[bg_count].command) - 1] = '\0';
        bg_count++;
    } else {
        printf(COLOR_RED "Maximum background process limit reached.\n" COLOR_RESET);
    }
}

void update_bg_process_state(pid_t pid, const char* state) {
    for (int i = 0; i < bg_count; i++) {
        if (bg_processes[i].pid == pid) {
            strncpy(bg_processes[i].state, state, sizeof(bg_processes[i].state) - 1);
            bg_processes[i].state[sizeof(bg_processes[i].state) - 1] = '\0';
            break;
        }
    }
}