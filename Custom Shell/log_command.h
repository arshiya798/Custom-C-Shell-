#ifndef LOG_COMMAND_H
#define LOG_COMMAND_H
void add_command_to_log(const char *command);
void purge_log();
void display_log();
void execute_command_from_log(int index);
void handle_log_command(char **args);

#endif // LOG_COMMAND_H
