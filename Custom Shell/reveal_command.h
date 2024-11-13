#ifndef REVEAL_COMMAND_H
#define REVEAL_COMMAND_H

void handle_reveal(char **args);
void reveal_directory(const char *path, int show_hidden, int long_format);

#endif // REVEAL_COMMAND_H
