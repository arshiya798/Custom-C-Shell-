# Compiler and flags
CC = gcc
CFLAGS = -lm -Wall

# Source files
SRC = main.c hop_command.c log_command.c process_input.c reveal_command.c seek_command.c pipe.c \
      proclore_command.c redirection.c system_commands.c signal_handling.c activities.c iMan.c fgbg.c myshrc.c alias.c neonate.c

# Output executable
OUTPUT = my_shell

# Default rule
all: $(OUTPUT)

# Linking object files to create executable
$(OUTPUT): $(SRC)
	$(CC) $(SRC) -o $(OUTPUT) $(CFLAGS)

# Clean rule to remove the executable
clean:
	rm -f $(OUTPUT)
