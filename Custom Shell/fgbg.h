#ifndef FGBG_H
#define FGBG_H

#include <sys/types.h>  // For pid_t

// Function to bring a process to the foreground
void fg(pid_t pid);

// Function to resume a stopped process in the background
void bg(pid_t pid);

int is_pid_in_bg_list(pid_t pid);
void remove_pid_from_bg_list(pid_t pid);
void update_bg_process_status(pid_t pid, const char *status);
int find_bg_process_index(pid_t pid);

#endif // FGBG_H
