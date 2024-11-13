#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <sys/select.h>  // For select()

// Red color escape codes
#define RED_TEXT "\033[31m"
#define RESET_TEXT "\033[0m"

void disable_ctrl_signals() {
    signal(SIGINT, SIG_IGN);  // Ignore Ctrl+C
    signal(SIGTSTP, SIG_IGN); // Ignore Ctrl+Z
}

void restore_terminal(struct termios *orig_termios) {
    tcsetattr(STDIN_FILENO, TCSANOW, orig_termios); // Restore original terminal settings
}

void enter_raw_mode(struct termios *orig_termios) {
    struct termios raw;
    tcgetattr(STDIN_FILENO, orig_termios); // Get current terminal settings
    raw = *orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &raw); // Set raw mode
}

pid_t get_most_recent_pid() {
    DIR *proc_dir;
    struct dirent *entry;
    pid_t last_pid = 0;

    proc_dir = opendir("/proc");
    if (proc_dir == NULL) {
        // Print error message in red
        fprintf(stderr, RED_TEXT "opendir failed: %s" RESET_TEXT "\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Iterate through the entries in /proc to find the largest PID
    while ((entry = readdir(proc_dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            pid_t pid = atoi(entry->d_name);
            if (pid > last_pid) {
                last_pid = pid;
            }
        }
    }

    closedir(proc_dir);
    return last_pid;
}

int check_for_x_key() {
    fd_set set;
    struct timeval timeout;
    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);  // Monitor standard input (keyboard)

    timeout.tv_sec = 0;  // No delay, check immediately
    timeout.tv_usec = 0; // No microsecond delay

    // Check if there's input available
    int rv = select(STDIN_FILENO + 1, &set, NULL, NULL, &timeout);
    if (rv == -1) {
        // Print error message in red
        fprintf(stderr, RED_TEXT "select failed: %s" RESET_TEXT "\n", strerror(errno));
        return 0;
    } else if (rv == 0) {
        return 0;  // No input detected
    } else {
        // There's input, check if it's 'x'
        char c = getchar();
        return c == 'x';
    }
}

void neonate_functionality(int interval) {
    disable_ctrl_signals(); // Disable Ctrl+C and Ctrl+Z

    struct termios orig_termios;
    enter_raw_mode(&orig_termios); // Enter raw mode

    while (1) {
        pid_t recent_pid = get_most_recent_pid();
        printf("%d\n", recent_pid);

        // Sleep for the specified interval
        sleep(interval);

        // Check for 'x' input to stop the program
        if (check_for_x_key()) {
            break;
        }
    }

    restore_terminal(&orig_termios); // Restore terminal to original state
}
