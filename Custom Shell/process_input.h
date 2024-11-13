#ifndef PROCESS_COMMAND_H
#define PROCESS_COMMAND_H

// Function to modify the input by adding ';' after every '&'
void modify_input(const char *input, char *modified_input);

// Function to parse the input into commands and execute them
void process_input(char *input);

// Function to trim and prepare a command for execution (trims whitespace and handles '&')
void trim_and_prepare_command_for_execution(char *command,int* is_background);

// Function to parse a command into a list of arguments
char** parse_command(  char* command);

void process_command(const char *command);

int is_whitespace_only(const char *str);

void execute_command(char **args, int is_background,const char* command);

void add_to_prompt(const char *extra);

int check_ampersand_followed_by_pipe(const char *str, int ampersand_pos);


#endif // PROCESS_COMMAND_H
