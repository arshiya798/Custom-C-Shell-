#include "myshrc.h"
#include "alias.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

// Red color escape codes
#define RED_TEXT "\033[31m"
#define RESET_TEXT "\033[0m"

void load_myshrc() {

    char myshrc_path[1024];
    strcpy(myshrc_path, "myshrc.txt");

    printf("Loading myshrc from: %s\n", myshrc_path);

    FILE *myshrc_file = fopen(myshrc_path, "r");
    if (myshrc_file == NULL) {
        fprintf(stderr, RED_TEXT "Error opening myshrc.txt: %s\n" RESET_TEXT, strerror(errno));
        return; // myshrc.txt file does not exist
    }

    char line[1024];
    while (fgets(line, sizeof(line), myshrc_file)) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        // Parse and define alias
        char *eq_pos = strchr(line, '=');
        if (eq_pos) {
            *eq_pos = '\0';
            define_alias(line, eq_pos + 1);
            continue;
        }

        // Expand and execute alias
        const char *expanded_command = expand_alias(line);
        if (expanded_command) {
            int status = system(expanded_command);
            if (status == -1) {
                fprintf(stderr, RED_TEXT "Error executing command '%s': %s\n" RESET_TEXT, expanded_command, strerror(errno));
            } else {
                printf("Command exited with status %d\n", WEXITSTATUS(status));
            }
        } else {
            int status = system(line);
            if (status == -1) {
                fprintf(stderr, RED_TEXT "Error executing command '%s': %s\n" RESET_TEXT, line, strerror(errno));
            } else {
                printf("Command exited with status %d\n", WEXITSTATUS(status));
            }
        }
    }

    fclose(myshrc_file);
}
