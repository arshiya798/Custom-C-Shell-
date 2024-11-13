#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include "reveal_command.h"

// Assuming the previous_directory is declared in globals.h and defined in one of the C files
extern char previous_directory[4096];

#define GREEN "\033[0;32m"
#define WHITE "\033[0;37m"
#define BLUE "\033[0;34m"
#define RED "\033[0;31m"
#define RESET "\033[0m"

void display_single_file_info(const char *filename, const struct stat *file_stat, int long_format) {
    // Print file type, permissions, and additional information for long format
    if (long_format) {
        printf((S_ISDIR(file_stat->st_mode)) ? "d" : "-");
        printf((file_stat->st_mode & S_IRUSR) ? "r" : "-");
        printf((file_stat->st_mode & S_IWUSR) ? "w" : "-");
        printf((file_stat->st_mode & S_IXUSR) ? "x" : "-");
        printf((file_stat->st_mode & S_IRGRP) ? "r" : "-");
        printf((file_stat->st_mode & S_IWGRP) ? "w" : "-");
        printf((file_stat->st_mode & S_IXGRP) ? "x" : "-");
        printf((file_stat->st_mode & S_IROTH) ? "r" : "-");
        printf((file_stat->st_mode & S_IWOTH) ? "w" : "-");
        printf((file_stat->st_mode & S_IXOTH) ? "x" : "-");

        printf(" %ld", file_stat->st_nlink);
        printf(" %s", getpwuid(file_stat->st_uid)->pw_name);
        printf(" %s", getgrgid(file_stat->st_gid)->gr_name);
        printf(" %5ld", file_stat->st_size);

        char time_str[32];
        strftime(time_str, sizeof(time_str), "%b %d %H:%M", localtime(&file_stat->st_mtime));
        printf(" %s", time_str);
    }

    // Print the filename with appropriate color
    if (S_ISDIR(file_stat->st_mode)) {
        printf(" " BLUE "%s" RESET "\n", filename);
    } else if (file_stat->st_mode & S_IXUSR) {
        printf(" " GREEN "%s" RESET "\n", filename);
    } else {
        printf(" " WHITE "%s" RESET "\n", filename);
    }
}

void display_file_info(const char *path, const char *name, int show_hidden, int long_format) {
    struct stat file_stat;
    char full_path[4096];

    snprintf(full_path, sizeof(full_path), "%s/%s", path, name);

    if (stat(full_path, &file_stat) == -1) {
        fprintf(stderr, RED "stat: %s\n" RESET, strerror(errno));
        return;
    }

    // Skip hidden files if show_hidden is not set
    if (!show_hidden && name[0] == '.') {
        return;
    }

    // Print file type, permissions, and additional information for long format
    if (long_format) {
        printf((S_ISDIR(file_stat.st_mode)) ? "d" : "-");
        printf((file_stat.st_mode & S_IRUSR) ? "r" : "-");
        printf((file_stat.st_mode & S_IWUSR) ? "w" : "-");
        printf((file_stat.st_mode & S_IXUSR) ? "x" : "-");
        printf((file_stat.st_mode & S_IRGRP) ? "r" : "-");
        printf((file_stat.st_mode & S_IWGRP) ? "w" : "-");
        printf((file_stat.st_mode & S_IXGRP) ? "x" : "-");
        printf((file_stat.st_mode & S_IROTH) ? "r" : "-");
        printf((file_stat.st_mode & S_IWOTH) ? "w" : "-");
        printf((file_stat.st_mode & S_IXOTH) ? "x" : "-");

        printf(" %ld", file_stat.st_nlink);
        printf(" %s", getpwuid(file_stat.st_uid)->pw_name);
        printf(" %s", getgrgid(file_stat.st_gid)->gr_name);
        printf(" %5ld", file_stat.st_size);

        char time_str[32];
        strftime(time_str, sizeof(time_str), "%b %d %H:%M", localtime(&file_stat.st_mtime));
        printf(" %s", time_str);
    }

    // Color coding based on file type
    if (S_ISDIR(file_stat.st_mode)) {
        printf(" " BLUE "%s" RESET "\n", name);
    } else if (file_stat.st_mode & S_IXUSR) {
        printf(" " GREEN "%s" RESET "\n", name);
    } else {
        printf(" " WHITE "%s" RESET "\n", name);
    }
}

int lexicographic_sort(const struct dirent **a, const struct dirent **b) {
    return strcasecmp((*a)->d_name, (*b)->d_name);
}

void reveal_directory(const char *path, int show_hidden, int long_format) {
    struct dirent **namelist;
    int n;
    int visible_count = 0; // Count of visible items based on the show_hidden flag

    // Use the custom comparator for lexicographic sorting (case-insensitive)
    n = scandir(path, &namelist, NULL, lexicographic_sort);
    if (n < 0) {
        fprintf(stderr, RED "scandir: %s\n" RESET, strerror(errno));
        return;
    }

    // Count the number of items to be printed based on the show_hidden flag
    for (int i = 0; i < n; i++) {
        if (show_hidden || namelist[i]->d_name[0] != '.') {
            visible_count++;
        }
    }

    // Print the number of items that will be displayed
    printf("Number of items: %d\n", visible_count);

    // Iterate in ascending order (from low to high) and display items
    for (int i = 0; i < n; i++) {
        if (show_hidden || namelist[i]->d_name[0] != '.') {
            display_file_info(path, namelist[i]->d_name, show_hidden, long_format);
        }
        free(namelist[i]);
    }
    free(namelist);
}

void handle_reveal(char **args) {
    int show_hidden = 0;
    int long_format = 0;
    char path[4096];
    struct stat path_stat;

    strcpy(path, ".");

    int show_prev_command_encountered = 0;

    for (int i = 1; args[i] != NULL; i++) {
        if (args[i][0] == '-' && args[i][1] == '\0') {
            show_prev_command_encountered = 1; 
        }
        else if (args[i][0] == '-' && args[i][1] != '\0') {
            for (int j = 1; args[i][j] != '\0'; j++) {
                if (args[i][j] == 'a') {
                    show_hidden = 1;
                } else if (args[i][j] == 'l') {
                    long_format = 1;
                }
            }
        }
        else {
            strcpy(path, args[i]);
        }
    }

    // Handle special symbols
    if (strcmp(path, "~") == 0) {
        strcpy(path, home_directory);
    } else if (show_prev_command_encountered == 1) {
        if (strlen(previous_directory) > 0) {
            // printf("previous directory before reveal command is %s\n", previous_directory);
            strcpy(path, previous_directory);
        } else {
            fprintf(stderr, RED "reveal: no previous directory\n" RESET);
            return;
        }
    }

    // Check if the path is a file or directory
    if (stat(path, &path_stat) == -1) {
        fprintf(stderr, RED "stat: %s\n" RESET, strerror(errno));
        return;
    }

    if (S_ISDIR(path_stat.st_mode)) {
        // Reveal the directory contents
        reveal_directory(path, show_hidden, long_format);
    } else if (S_ISREG(path_stat.st_mode)) {
        // Extract the file name from the full path
        char *file_name = strrchr(path, '/');
        if (file_name) {
            file_name++; // Move past the '/' character to get the filename
        } else {
            file_name = path; // If there's no '/' in the path, the path is already the filename
        }

        // Print the details of the file using the new function
        display_single_file_info(file_name, &path_stat, long_format);
    } else {
        fprintf(stderr, RED "reveal: %s is neither a file nor a directory\n" RESET, path);
    }
}
