#include "alias.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ALIASES 100

static Alias aliases[MAX_ALIASES];
static int alias_count = 0;

void define_alias(const char *name, const char *value) {
    if (alias_count < MAX_ALIASES) {
        strncpy(aliases[alias_count].name, name, MAX_ALIAS_NAME - 1);
        strncpy(aliases[alias_count].value, value, MAX_ALIAS_LENGTH - 1);
        alias_count++;
    }
}

const char *expand_alias(const char *name) {
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(aliases[i].name, name) == 0) {
            return aliases[i].value;
        }
    }
    return NULL;
}
