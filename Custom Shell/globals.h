// globals.h
#ifndef GLOBALS_H
#define GLOBALS_H

#include <sys/types.h>  // Add this line to define pid_t
#include <unistd.h> 

#define MAX_BG_PROCESSES 4096

extern char username[256];
extern char systemname[256];
extern char previous_directory[4096];
extern char home_directory[4096];
extern int log_count;
extern int next_index;
extern char command_log[15][1024];
extern char log_file_path[5100];
extern char foreground_command[4096];
extern char prompt_extras[4096];

// Structure to store background process information
typedef struct {
    pid_t pid;
    char command[4096];
    char state[16];
} BackgroundProcess;

// Global array to store background processes
extern BackgroundProcess bg_processes[MAX_BG_PROCESSES];
extern int bg_count;

// Declaration of the global variable for the foreground process PID
extern pid_t foreground_pid;

#endif
