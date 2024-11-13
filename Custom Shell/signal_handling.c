#include "signal_handling.h"
#include "globals.h"
#include "system_commands.h"
#include <string.h>
#include <errno.h>

// Red color escape codes
#define RED_TEXT "\033[31m"
#define RESET_TEXT "\033[0m"

// Function to handle sending signals from the 'ping' command
void ping_process(pid_t pid, int signal_number) {
    int mod_signal = signal_number % 32;

    if (mod_signal == 15) {  // SIGTERM
        send_SIGTERM(pid);
    } 
    else if (mod_signal == 2) {  // SIGINT
        send_SIGINT(pid);
    } 
    else if (mod_signal == 19) {  // SIGSTOP
        send_SIGSTOP(pid);
    } 
    else if (mod_signal == 18) {  // SIGCONT
        send_SIGCONT(pid);
    } 
    else if (mod_signal == 9) {  // SIGKILL
        send_SIGKILL(pid);
    } 
    else {
        // Handle other signals or print an error message
        if (kill(pid, mod_signal) == -1) {
            fprintf(stderr, RED_TEXT "Error sending signal %d to process %d: %s\n" RESET_TEXT, mod_signal, pid, strerror(errno));
        } else {
            printf("Sent signal %d to process with pid %d\n", mod_signal, pid);
        }
    }
}

// Function to handle Ctrl+C (SIGINT)
void handle_sigint(int sig) {
    if (foreground_pid > 0) {
        // Send SIGINT to the foreground process
        if (kill(foreground_pid, SIGINT) == -1) {
            fprintf(stderr, RED_TEXT "Error sending SIGINT to process %d: %s\n" RESET_TEXT, foreground_pid, strerror(errno));
        }
        // Reset foreground_pid after sending the signal
        foreground_pid = -1;
        strcpy(foreground_command, "");
    }
}

void handle_sigtstp(int sig) {
    if (foreground_pid != -1) {  // Check if a foreground process is running

        // Move the process to the background and update its state to "Stopped"
        add_command_to_bg_list(foreground_command, foreground_pid);  // You can modify this to track the actual command if needed
        update_bg_process_state(foreground_pid, "Stopped");
        foreground_pid = -1;

        // Send the SIGTSTP signal to the foreground process
        if (kill(foreground_pid, SIGTSTP) == -1) {
            fprintf(stderr, RED_TEXT "Error sending SIGTSTP to process %d: %s\n" RESET_TEXT, foreground_pid, strerror(errno));
        }

        printf("Foreground process [%d] has been stopped and moved to the background.\n", foreground_pid);

        // Reset foreground_pid since there's no longer an active foreground process
        
    }
}

// Function to handle Ctrl+D (EOF) and exit shell
void handle_eof() {
    printf("\nCtrl+D detected. Exiting shell...\n");
    // Terminate the foreground process
    if (foreground_pid > 0) {
        terminate_process(foreground_pid);
        foreground_pid = -1;  // Reset foreground_pid after termination
        strcpy(foreground_command, "");
    }

    // Terminate all background processes
    for (int i = 0; i < bg_count; i++) {
        if (bg_processes[i].pid > 0) {
            terminate_process(bg_processes[i].pid);
        }
    }

    // Reset background process count
    bg_count = 0;

    // Exit the shell gracefully
    exit(0);
}

// Function to terminate a process gracefully
void terminate_process(int pid) {
    if (kill(pid, SIGTERM) == -1) {
        fprintf(stderr, RED_TEXT "Error sending SIGTERM to process %d: %s\n" RESET_TEXT, pid, strerror(errno));
        return;
    }
    sleep(1);  // Give the process a moment to terminate gracefully

    // Check if the process is still running
    int status;
    if (waitpid(pid, &status, WNOHANG) == 0) {
        // Process is still running; send SIGKILL
        printf("Process [%d] did not terminate; sending SIGKILL...\n", pid);
        if (kill(pid, SIGKILL) == -1) {
            fprintf(stderr, RED_TEXT "Error sending SIGKILL to process %d: %s\n" RESET_TEXT, pid, strerror(errno));
            return;
        }
        waitpid(pid, NULL, 0);  // Wait for the process to terminate
    }
}

// Function to terminate a process using SIGTERM
void send_SIGTERM(pid_t pid) {
    if (is_pid_in_bg_list(pid)) {
        // Send SIGTERM to terminate the background process
        if (kill(pid, SIGTERM) == -1) {
            fprintf(stderr, RED_TEXT "Error sending SIGTERM to process %d: %s\n" RESET_TEXT, pid, strerror(errno));
            return;
        }
        printf("Sent signal %d to process with pid %d\n", SIGTERM, pid);

        sleep(1); // Give the process a moment to terminate gracefully

        // Check if the process is still running
        int status;
        if (waitpid(pid, &status, WNOHANG) != 0) {
            remove_pid_from_bg_list(pid);
        }
    }
    else if (pid == foreground_pid) {
        // Terminate the foreground process
        if (kill(foreground_pid, SIGTERM) == -1) {
            fprintf(stderr, RED_TEXT "Error sending SIGTERM to process %d: %s\n" RESET_TEXT, foreground_pid, strerror(errno));
            return;
        }
        printf("Sent signal %d to process with pid %d\n", SIGTERM, foreground_pid);

        sleep(1); // Give the process a moment to terminate gracefully

        // Check if the process is still running
        int status;
        if (waitpid(foreground_pid, &status, WNOHANG) != 0) {
            foreground_pid = -1;
            strcpy(foreground_command, "");
        }
    }
}

// Function to terminate a process forcefully using SIGKILL
void send_SIGKILL(pid_t pid) {
    if (is_pid_in_bg_list(pid)) {
        // Send SIGKILL to forcefully terminate the background process
        if (kill(pid, SIGKILL) == -1) {
            fprintf(stderr, RED_TEXT "Error sending SIGKILL to process %d: %s\n" RESET_TEXT, pid, strerror(errno));
            return;
        }
        printf("Sent signal %d to process with pid %d\n", SIGKILL, pid);

        // Remove the process from the background list
        remove_pid_from_bg_list(pid);
    }
    else if (pid == foreground_pid) {
        // Terminate the foreground process
        if (kill(foreground_pid, SIGKILL) == -1) {
            fprintf(stderr, RED_TEXT "Error sending SIGKILL to process %d: %s\n" RESET_TEXT, foreground_pid, strerror(errno));
            return;
        }
        printf("Sent signal %d to process with pid %d\n", SIGKILL, foreground_pid);

        // Reset foreground_pid and clear the command
        foreground_pid = -1;
        strcpy(foreground_command, "");
    }
}

// Function to send SIGINT to a process
void send_SIGINT(pid_t pid) {
    if (is_pid_in_bg_list(pid)) {
        // Send SIGINT to interrupt the background process
        if (kill(pid, SIGINT) == -1) {
            fprintf(stderr, RED_TEXT "Error sending SIGINT to process %d: %s\n" RESET_TEXT, pid, strerror(errno));
            return;
        }
        printf("Sent signal %d to process with pid %d\n", SIGINT, pid);

        // Update the background process status to "Stopped"
        update_bg_process_status(pid, "Stopped");
    }
    else if (pid == foreground_pid) {
        // Interrupt the foreground process
        if (kill(foreground_pid, SIGINT) == -1) {
            fprintf(stderr, RED_TEXT "Error sending SIGINT to process %d: %s\n" RESET_TEXT, foreground_pid, strerror(errno));
            return;
        }
        printf("Sent signal %d to process with pid %d\n", SIGINT, foreground_pid);
    }
}

// Function to stop a process using SIGSTOP
void send_SIGSTOP(pid_t pid) {
    if (is_pid_in_bg_list(pid)) {
        // Send SIGSTOP to stop the background process
        if (kill(pid, SIGSTOP) == -1) {
            fprintf(stderr, RED_TEXT "Error sending SIGSTOP to process %d: %s\n" RESET_TEXT, pid, strerror(errno));
            return;
        }
        printf("Sent signal %d to process with pid %d\n", SIGSTOP, pid);

        // Update the background process status to "Stopped"
        update_bg_process_status(pid, "Stopped");
    }
    else if (pid == foreground_pid) {
        // Stop the foreground process
        if (kill(foreground_pid, SIGSTOP) == -1) {
            fprintf(stderr, RED_TEXT "Error sending SIGSTOP to process %d: %s\n" RESET_TEXT, foreground_pid, strerror(errno));
            return;
        }
        printf("Sent signal %d to process with pid %d\n", SIGSTOP, foreground_pid);
    }
}

// Function to continue a stopped process using SIGCONT
void send_SIGCONT(pid_t pid) {
    if (is_pid_in_bg_list(pid)) {
        // Send SIGCONT to continue the background process
        if (kill(pid, SIGCONT) == -1) {
            fprintf(stderr, RED_TEXT "Error sending SIGCONT to process %d: %s\n" RESET_TEXT, pid, strerror(errno));
            return;
        }
        printf("Sent signal %d to process with pid %d\n", SIGCONT, pid);

        // Update the background process status to "Running"
        update_bg_process_status(pid, "Running");
    }
    else if (pid == foreground_pid) {
        // Continue the foreground process
        if (kill(foreground_pid, SIGCONT) == -1) {
            fprintf(stderr, RED_TEXT "Error sending SIGCONT to process %d: %s\n" RESET_TEXT, foreground_pid, strerror(errno));
            return;
        }
        printf("Sent signal %d to process with pid %d\n", SIGCONT, foreground_pid);
    }
}
