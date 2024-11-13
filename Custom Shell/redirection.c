#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h> 
#include <fcntl.h>  
#include "redirection.h"

// Red color escape codes
#define RED_TEXT "\033[31m"
#define RESET_TEXT "\033[0m"

// Function to remove a substring from a string
char* remove_substring(char* str, char* start, char* end) {
    if (str == NULL || start == NULL || end == NULL || start >= end) {
        return str;
    }

    size_t prefix_len = start - str;
    size_t suffix_len = strlen(end);  

    memmove(start, end, suffix_len + 1);  
    return str;
}

// Function to handle redirections in a command
void handle_redirections(char *command, char **input_file, char **output_file, int *append) {
    *input_file = NULL;
    *output_file = NULL;
    *append = 0; 

    char *input_redir = strchr(command, '<');

    if (input_redir) {
        char *filename_start = input_redir + 1;

        while (isspace((unsigned char)*filename_start)) {
            filename_start++;
        }

        char *filename_end = filename_start;
        while (*filename_end && !isspace((unsigned char)*filename_end)) {
            filename_end++;
        }

        size_t filename_length = filename_end - filename_start;
        *input_file = (char *)malloc(filename_length + 1);
        if (*input_file == NULL) {
            fprintf(stderr, RED_TEXT "Memory allocation error" RESET_TEXT "\n");
            exit(EXIT_FAILURE);
        }
        strncpy(*input_file, filename_start, filename_length);
        (*input_file)[filename_length] = '\0';

        // Check if input file exists
        if (access(*input_file, F_OK) != 0) {
            // Print error message in red
            fprintf(stderr, RED_TEXT "No such input file found: %s" RESET_TEXT "\n", *input_file);
            free(*input_file);
            *input_file = NULL;  
        }

        command = remove_substring(command, input_redir, filename_end);
    }

    char *output_redir = strstr(command, ">>");

    if (output_redir) {
        *append = 1;
    } else {
        output_redir = strchr(command, '>');
    }

    if (output_redir) {
        char *filename_start = (*append) ? output_redir + 2 : output_redir + 1;

        while (isspace((unsigned char)*filename_start)) {
            filename_start++;
        }

        char *filename_end = filename_start;
        while (*filename_end && !isspace((unsigned char)*filename_end)) {
            filename_end++;
        }

        size_t filename_length = filename_end - filename_start;
        *output_file = (char *)malloc(filename_length + 1);
        if (*output_file == NULL) {
            fprintf(stderr, RED_TEXT "Memory allocation error" RESET_TEXT "\n");
            exit(EXIT_FAILURE);
        }
        strncpy(*output_file, filename_start, filename_length);
        (*output_file)[filename_length] = '\0';

        command = remove_substring(command, output_redir, filename_end);
    }
    
    setup_io_redirection(*input_file, *output_file, append);
}

// Function to set up I/O redirection
void setup_io_redirection(char *input_file, char *output_file, int* append) {
    if (input_file) {
        FILE *input_fd = fopen(input_file, "r");
        if (!input_fd) {
            // Print error message in red
            fprintf(stderr, RED_TEXT "Error opening input file: %s" RESET_TEXT "\n", input_file);
            exit(EXIT_FAILURE);
        }
        dup2(fileno(input_fd), STDIN_FILENO); 
        fclose(input_fd);
    }

    if (output_file) {
        int flags = O_WRONLY | O_CREAT;
        flags |= (*append) ? O_APPEND : O_TRUNC;  

        int output_fd = open(output_file, flags, 0644);
        if (output_fd < 0) {
            // Print error message in red
            fprintf(stderr, RED_TEXT "Error opening output file: %s" RESET_TEXT "\n", output_file);
            exit(EXIT_FAILURE);
        }
        dup2(output_fd, STDOUT_FILENO);
        close(output_fd);
    }
}
