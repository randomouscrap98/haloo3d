#include <string.h>
#include <ctype.h>

#include "mystring.h"

void string_trim(char *str) {
    if (!str) return;

    // Trim leading whitespace
    while (isspace((unsigned char)*str)) {
        str++;
    }

    // If string is now empty, return
    if (*str == '\0') return;

    // Trim trailing whitespace
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0';
}

