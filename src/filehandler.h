#ifndef FILE_H
#define FILE_H
#include "common.h"

text* read_file(const char* filename);

// returns char that was in space of whats gone in
void ins_char(text* txt, cursor* cur, char chr);

// always returns \n
void ins_newline(text* txt, cursor* cur);

// returns deleted char
char del_char(text* txt, cursor* cur);

char parse_args(int argc, char** argv);

int copy_file(const char* source, const char* target);

int save_file(const char* filename, const text* txt);

int delete_file(const char* filename);

text* create_file(const char* filename);

int words(const char* txt);

int write_log(const char* text);

#endif