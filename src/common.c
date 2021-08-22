#include "common.h"

#include <string.h>

/*
gets the distance (how many chars) from *ptr to the end of the line
tries to strchr it but if this NULLS (ie there is no newline char), then
uses strlen instead to count up to a \0
*/
int line_length(const char* ptr) {
    if (strchr(ptr, '\n') == NULL) {
        return strlen(ptr);
    }
    return strchr(ptr, '\n') - ptr;
}

/*
returns a pointer to the charachter that is currently selected by the cursor
*/
char* get_char(const cursor* cur, const text* txt) {
    // init pointer to head of buffer
    char* ptr = txt->buffer;
    // loop over lines up to line offset+y
    for (int i = 1; i < cur->line_offset + cur->y; i++)
        ptr += line_length(ptr) + 1;
    // add the column offset and x to the pointer

    ptr += cur->column_offset + cur->x;
    return ptr;
}