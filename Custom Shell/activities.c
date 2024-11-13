#include "globals.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int compare_bg_processes(const void* a, const void* b) {
    const BackgroundProcess *proc1 = (const BackgroundProcess*)a;
    const BackgroundProcess *proc2 = (const BackgroundProcess*)b;
    return strcmp(proc1->command, proc2->command);
}

void display_background_processes() {
    qsort(bg_processes, bg_count, sizeof(BackgroundProcess), compare_bg_processes);

    for (int i = 0; i < bg_count; i++) {
        printf("%d : %s - %s\n", bg_processes[i].pid, bg_processes[i].command, bg_processes[i].state);
    }
}