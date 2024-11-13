#ifndef REDIRECTION_H
#define REDIRECTION_H

// Function to remove a substring from a string, given pointers to the start and end of the substring
char* remove_substring(char* str, char* start, char* end);

// Function to handle input/output redirections, setting file paths and append mode
void handle_redirections(char *command, char **input_file, char **output_file, int *append);

void setup_io_redirection(char *input_file, char *output_file, int* append);

#endif // REDIRECTION_H
