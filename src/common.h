#ifndef COMMON_H
#define COMMON_H

// amount of text that can fit in text box
// defined as macros for global usage
#define T_LINES 26
#define T_COLUMNS 116

// represents the cursor location
// x and y are location within textview windows
// line and column offset are how much the textview is offset by in relation to the file
typedef struct {
    int x;
    int y;
    int line_offset;
    int column_offset;
} cursor;

// struct to store file text
// store buffer size and string length so dynamic reallocation is much easier
// also store number of lines because we need to keep track of that
typedef struct {
    char* buffer;
    int length;
    int bufsize;
    int lines;
} text;

int line_length(const char* ptr);
char* get_char(const cursor* cur, const text* txt);
#endif