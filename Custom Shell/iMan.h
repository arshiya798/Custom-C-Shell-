#ifndef IMAN_H
#define IMAN_H

#include <stddef.h> // For size_t
#include <stdio.h>  // For FILE type and related functions

#define PORT 80
#define MAX_BUFFER_SIZE 8192

// Struct to hold the man page content
typedef struct {
    char content[MAX_BUFFER_SIZE];
    size_t length;
} ManPage;

// Function to fetch the man page for a given command
void fetch_man_page(const char *command, ManPage *man_page);

#endif // IMAN_H
