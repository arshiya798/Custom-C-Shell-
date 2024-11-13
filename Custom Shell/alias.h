#ifndef ALIAS_H
#define ALIAS_H

#define MAX_ALIAS_LENGTH 256
#define MAX_ALIAS_NAME 64

typedef struct {
    char name[MAX_ALIAS_NAME];
    char value[MAX_ALIAS_LENGTH];
} Alias;

void define_alias(const char *name, const char *value);
const char *expand_alias(const char *name);

#endif // ALIAS_H
