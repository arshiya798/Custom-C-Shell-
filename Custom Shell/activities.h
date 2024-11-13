#ifndef ACTIVITIES_H
#define ACTIVITIES_H

#include "globals.h"  // Ensure this includes the definition of BackgroundProcess and any other required definitions

// Function to compare two background processes for sorting
int compare_bg_processes(const void* a, const void* b);

// Function to display all background processes
void display_background_processes(void);

#endif // ACTIVITIES_H
