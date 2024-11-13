#ifndef SYSTEM_COMMANDS_H
#define SYSTEM_COMMANDS_H

#include <sys/types.h>


// Function prototypes
void add_command_to_bg_list(const char *command,pid_t pid);
void execute_system_command(char **args, int is_background,const char* command);
void check_background_processes(void);
void update_bg_process_state(pid_t pid, const char* state) ;

#endif // SYSTEM_COMMANDS_H
