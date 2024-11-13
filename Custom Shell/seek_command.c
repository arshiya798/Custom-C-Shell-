#include "seek_command.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#define COLOR_GREEN "\033[0;32m"
#define COLOR_BLUE "\033[0;34m"
#define COLOR_RED "\033[0;31m"
#define COLOR_RESET "\033[0m"

// Function to recursively search for files and directories with a prefix match
int search_directory(const char *base_path, const char *target_name, int search_files, int search_dirs, int execute_flag, int *match_count, char *single_match) {
    struct dirent *dp;
    DIR *dir = opendir(base_path);

    // Return if unable to open directory
    if (!dir) {
        return 0;
    }

    while ((dp = readdir(dir)) != NULL) {
        char path[1024];
        struct stat statbuf;

        // Skip "." and ".." directories
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
            continue;
        }

        snprintf(path, sizeof(path), "%s/%s", base_path, dp->d_name);

        // Get file status
        stat(path, &statbuf);

        // Check if the current entry's name starts with the target name
        if (strncmp(dp->d_name, target_name, strlen(target_name)) == 0) {
            int is_directory = S_ISDIR(statbuf.st_mode);
            int is_file = S_ISREG(statbuf.st_mode);

            if ((is_directory && search_dirs) || (is_file && search_files)) {
                if (is_directory) {
                    printf(COLOR_BLUE "%s\n" COLOR_RESET, path);
                } else if (is_file) {
                    printf(COLOR_GREEN "%s\n" COLOR_RESET, path);
                }

                (*match_count)++;

                if (*match_count == 1) {
                    strcpy(single_match, path);
                }
            }
        }

        // If it's a directory, recursively search it
        if (S_ISDIR(statbuf.st_mode)) {
            search_directory(path, target_name, search_files, search_dirs, execute_flag, match_count, single_match);
        }
    }

    closedir(dir);
    return 1;
}

// Function to handle the "seek" command
void handle_seek(char **args) {
    int search_files = 1;
    int search_dirs = 1;
    int execute_flag = 0;
    char *target_name = NULL;
    char *target_dir = ".";
    
    // Parse flags
    int i = 1;
    int flag_d = 0;
    int flag_f = 0;

    while (args[i] && args[i][0] == '-') {
        if (strcmp(args[i], "-d") == 0) {
            flag_d = 1;
            search_files = 0;
            search_dirs = 1;
        } else if (strcmp(args[i], "-f") == 0) {
            flag_f = 1;
            search_files = 1;
            search_dirs = 0;
        } else if (strcmp(args[i], "-e") == 0) {
            execute_flag = 1;
        } else {
            printf(COLOR_RED "Invalid flags!\n" COLOR_RESET);
            return;
        }
        i++;
    }

    // Ensure that -d and -f are not used together
    if (flag_d && flag_f) {
        printf(COLOR_RED "Invalid flags!\n" COLOR_RESET);
        return;
    }

    // Extract target name and directory
    if (args[i]) {
        target_name = args[i];
        i++;
    } else {
        printf(COLOR_RED "seek: missing search target\n" COLOR_RESET);
        return;
    }

    if (args[i]) {
        target_dir = args[i];
    }

    // Check if the target directory exists
    struct stat statbuf;
    if (stat(target_dir, &statbuf) != 0 || !S_ISDIR(statbuf.st_mode)) {
        printf(COLOR_RED "Target directory does not exist!\n" COLOR_RESET);
        return;
    }

    int match_count = 0;
    char single_match[1024] = "";

    // Start searching from the target directory
    search_directory(target_dir, target_name, search_files, search_dirs, execute_flag, &match_count, single_match);

    // If no match was found, print "No match found!"
    if (match_count == 0) {
        printf(COLOR_RED "No match found!\n" COLOR_RESET);
    }

    // Handle -e flag functionality after search is complete
    if (execute_flag && match_count == 1) {
        struct stat final_stat;
        stat(single_match, &final_stat);

        if (search_dirs && S_ISDIR(final_stat.st_mode)) {
            if (chdir(single_match) != 0) {
                printf(COLOR_RED "Missing permissions for task!\n" COLOR_RESET);
            }
        } else if (search_files && S_ISREG(final_stat.st_mode)) {
            FILE *file = fopen(single_match, "r");
            if (!file) {
                printf(COLOR_RED "Missing permissions for task!\n" COLOR_RESET);
            } else {
                char c;
                while ((c = fgetc(file)) != EOF) {
                    putchar(c);
                }
                fclose(file);
            }
        }
    }
}
