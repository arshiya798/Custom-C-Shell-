#ifndef SIGNAL_HANDLING_H
#define SIGNAL_HANDLING_H

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


// Function to handle Ctrl+C
void handle_sigint(int sig);

// Function to handle Ctrl+Z
void handle_sigtstp(int sig);

// Function to handle Ctrl+D (EOF) - Custom implementation
void handle_eof(void);

void terminate_process(pid_t pid);

// Function to handle sending signals from the 'ping' command
void ping_process(pid_t pid, int signal_number);

void send_SIGTERM(pid_t pid);
void send_SIGINT(pid_t pid);
void send_SIGSTOP(pid_t pid);
void send_SIGCONT(pid_t pid);
void send_SIGKILL(pid_t pid);

static int is_pid_in_bg_list(pid_t pid);
static void remove_pid_from_bg_list(pid_t pid);
static void update_bg_process_status(pid_t pid, const char *status);
static int find_bg_process_index(pid_t pid);

#endif // SIGNAL_HANDLING_H
