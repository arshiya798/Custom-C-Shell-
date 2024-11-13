#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include "proclore_command.h"
#include <unistd.h>  // For readlink


// Define ANSI color codes for red text and reset color
#define RED_COLOR "\033[31m"
#define RESET_COLOR "\033[0m"

// Function to get the process status string based on the status character
const char* get_process_status(char status) {
    if (status == 'R') {
        return "R+";
    } else if (status == 'S') {
            return "S";
    } else if (status == 'Z') {
        return "Z";
    } else {
        return "Unknown";
    }
}
int is_bg_process(pid_t pid) {
    for (int i = 0; i < bg_count; ++i) {
        if (bg_processes[i].pid == pid) {
            return 1;  // Found in background processes
        }
    }
    return 0;  // Not found
}

// Function to obtain process information
void handle_proclore(char **args) {
    pid_t pid = getpid(); // Default to current shell process
    if (args[1] != NULL) {
        pid = atoi(args[1]); // Use PID from argument if provided
    }

    char path[4096];
    snprintf(path, sizeof(path), "/proc/%d/status", pid);

    FILE *status_file = fopen(path, "r");
    if (status_file == NULL) {
        fprintf(stderr, RED_COLOR "Error opening file %s: %s\n" RESET_COLOR, path, strerror(errno));
        return;
    }

    char line[256];
    char executable_path[4096] = "Unknown";
    char process_status[32] = "Unknown";
    int process_group = -1;
    long virtual_memory = -1;

    while (fgets(line, sizeof(line), status_file)) {
        if (strncmp(line, "State:", 6) == 0) {
            char state;
            sscanf(line, "State: %c", &state);
            snprintf(process_status, sizeof(process_status), "%s", get_process_status(state));
        } else if (strncmp(line, "PPid:", 5) == 0) {
            int ppid;
            sscanf(line, "PPid: %d", &ppid);
            process_group = ppid; // Assuming process group is parent PID for simplicity
        } else if (strncmp(line, "VmSize:", 7) == 0) {
            sscanf(line, "VmSize: %ld kB", &virtual_memory);
        }
    }

    fclose(status_file);

    // Get the executable path
    snprintf(path, sizeof(path), "/proc/%d/exe", pid);
    ssize_t len = readlink(path, executable_path, sizeof(executable_path) - 1);
    if (len == -1) {
        fprintf(stderr, RED_COLOR "Error reading symbolic link %s: %s\n" RESET_COLOR, path, strerror(errno));
        return;
    }
    executable_path[len] = '\0';

    printf("pid : %d\n", pid);
    printf("process status : %s\n", process_status);
    printf("Process Group : %d\n", process_group);
    printf("Virtual memory : %ld\n", virtual_memory);
    printf("executable path : %s\n", executable_path);
}
