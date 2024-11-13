#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include "fgbg.h"
#include "globals.h" // Include your globals header

// Red color escape codes
#define RED_TEXT "\033[31m"
#define RESET_TEXT "\033[0m"

void fg(pid_t pid) {
    // Check if the process exists by sending signal 0 (does not affect the process)
    if (kill(pid, 0) == -1) {
        if (errno == ESRCH) {
            printf(RED_TEXT "No such process found\n" RESET_TEXT);
            return;
        } else if (errno == EPERM) {
            printf(RED_TEXT "No permission to control the process\n" RESET_TEXT);
            return;
        }
        fprintf(stderr, RED_TEXT "Error checking process existence: %s\n" RESET_TEXT, strerror(errno));
        return;
    }

    // Send SIGCONT to resume the process if it was stopped
    if (kill(pid, SIGCONT) == -1) {
        fprintf(stderr, RED_TEXT "kill(SIGCONT) failed: %s\n" RESET_TEXT, strerror(errno));
        return;
    }

    // Wait for the process to terminate or stop again
    int status;
    if (waitpid(pid, &status, WUNTRACED) == -1) {
        fprintf(stderr, RED_TEXT "waitpid failed: %s\n" RESET_TEXT, strerror(errno));
        return;
    }

    // Remove the process from the background list
    for (int i = 0; i < bg_count; i++) {
        if (bg_processes[i].pid == pid) {
            // Shift all processes after the removed one
            for (int j = i; j < bg_count - 1; j++) {
                bg_processes[j] = bg_processes[j + 1];
            }
            bg_count--; // Decrease the count of background processes
            break;
        }
    }
}

void bg(pid_t pid) {
    // Check if the process exists by sending signal 0 (does not affect the process)
    if (kill(pid, 0) == -1) {
        if (errno == ESRCH) {
            printf(RED_TEXT "No such process found\n" RESET_TEXT);
            return;
        }
    }

    // Send SIGCONT to resume the process in the background
    if (kill(pid, SIGCONT) == -1) {
        fprintf(stderr, RED_TEXT "kill(SIGCONT) failed: %s\n" RESET_TEXT, strerror(errno));
        return;
    }

    // Update the status of the background process in the bg list
    int bg_index = find_bg_process_index(pid);  // Function to find process index in the bg list
    if (bg_index != -1) {
        update_bg_process_status(pid, "Running");
    } else {
        printf(RED_TEXT "Process %d is not in the background process list\n" RESET_TEXT, pid);
    }
}

int is_pid_in_bg_list(pid_t pid) {
    return find_bg_process_index(pid) != -1;
}

void remove_pid_from_bg_list(pid_t pid) {
    int index = find_bg_process_index(pid);
    if (index != -1) {
        // Shift all elements after the index to the left
        for (int i = index; i < bg_count - 1; i++) {
            bg_processes[i] = bg_processes[i + 1];
        }
        bg_count--; // Decrease the count of background processes
    }
}

// Function to update the status of a background process
void update_bg_process_status(pid_t pid, const char *status) {
    int index = find_bg_process_index(pid);
    if (index != -1) {
        strncpy(bg_processes[index].state, status, sizeof(bg_processes[index].state) - 1);
        bg_processes[index].state[sizeof(bg_processes[index].state) - 1] = '\0'; // Ensure null-termination
    }
}

int find_bg_process_index(pid_t pid) {
    for (int i = 0; i < bg_count; i++) {
        if (bg_processes[i].pid == pid) {
            return i;
        }
    }
    return -1; // Not found
}
