#ifndef PIPE_H
#define PIPE_H

#define MAX_SUBCOMMANDS 20

// Function to parse a command with piping into subcommands

void parse_piping_command(const char* command, char* subcommands[],int is_background);

// Function to handle the overall piping process and execute piped commands
void handle_piping_command(const char *command, int is_background);

// Function to execute piped subcommands using multiple processes
void execute_piped_commands(char* subcommands[], int num_commands, int is_background);

#endif // PIPE_H
