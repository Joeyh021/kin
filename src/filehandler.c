#include "filehandler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/*
loads a file with a given filename into memory, and creates a text structure
containing all the relevant info about it. returns pointer to the struct
allocated on heap.
*/
text* read_file(const char* filename) {
    // allocate some memory
    text* t = malloc(sizeof(text));
    FILE* fp;

    // open file
    if (!(fp = fopen(filename, "r")))
        return NULL;
    // seek to the end of the file to get the length of it
    fseek(fp, 0, SEEK_END);
    t->length = ftell(fp);
    rewind(fp);

    // make the buffer slightly larger by default
    t->bufsize = (int)(t->length * 1.2);
    if (t->bufsize < 10)
        t->bufsize = 10;

    // allocate this memory
    t->buffer = malloc(t->bufsize);

    // read the text into memory
    fread(t->buffer, 1, t->length, fp);

    // dont forget to close your files !
    fclose(fp);

    // null terminate the file
    // this is important because files with no trailing newlines will
    // just end in a plain charachter, which is bad
    // because then we have no idea where the end of the file is.
    t->buffer[t->length] = '\0';

    // count lines
    char* lptr = t->buffer;
    t->lines = 1;
    while ((lptr = strchr(lptr + 1, '\n')))
        t->lines++;

    // return our pointer
    return t;
}

// inserts a charachter chr into the buffer at the current cursor position cur
void ins_char(text* txt, cursor* cur, char chr) {
    // realloc buffer if we're gonna need to
    if (txt->length + 2 >= txt->bufsize) {
        char* t = malloc((int)(txt->bufsize * 1.5));
        memcpy(t, txt->buffer, txt->bufsize);
        free(txt->buffer);
        txt->buffer = t;
        txt->bufsize = (int)(txt->bufsize * 1.5);
    }

    // get a pointer to the position where we need to put the charachter
    char* charptr = get_char(cur, txt);

    // move everything to the left of where we're inserting up by one to make room
    // plus 1 because if we accidentaly overwite the null terminator its segfault city
    memcpy(charptr + 1, charptr, txt->length + 1 - (charptr - txt->buffer));

    // insert
    *charptr = chr;

    // move the cursor to the left by one, and if we need to increase column offset then do that too.
    cur->x++;
    if (cur->x > T_COLUMNS) {
        cur->x = T_COLUMNS;
        cur->column_offset++;
    }

    txt->length++;
}

// inserts a newline charachter at the current cursor position cur
void ins_newline(text* txt, cursor* cur) {
    // literally mostly the same as function above

    if (txt->length + 2 >= txt->bufsize) {
        char* t = malloc((int)(txt->bufsize * 1.5));
        memcpy(t, txt->buffer, txt->bufsize);
        free(txt->buffer);
        txt->buffer = t;
        txt->bufsize = (int)(txt->bufsize * 1.5);
    }

    char* charptr = get_char(cur, txt);

    memcpy(charptr + 1, charptr, txt->length - (charptr - txt->buffer));

    *charptr = '\n';

    // differs in the way we move the cursor
    // need to move cursor down by one
    cur->y++;
    if (cur->y > T_LINES) {
        cur->y = T_LINES;
        cur->line_offset++;
    }
    // and also to the start of the line
    cur->x = 0;
    cur->column_offset = 0;

    txt->lines++;
    txt->length++;
}

/*
counts the number of words in a file, up the null terminator, and returns this value.
a "word" is counted every time a newline, space or terminator is found that is preceded by another charachter that
isnt one of those three.
*/
int words(const char* txt) {
    int count = 0;
    int i = 0;
    // loop over the file, break on a \0, increment on a \n or a space
    while (1) {
        if (txt[i] == '\n' || txt[i] == ' ' || txt[i] == '\0')
            if (txt[i] != 0 && txt[i - 1] != '\n' && txt[i - 1] != ' ')
                count++;
        if (txt[i] == '\0') {
            if (txt[i] != 0 && txt[i - 1] != '\n' && txt[i - 1] != ' ')
                count++;
            return count;
        }
        i++;
    }
}

char del_char(text* txt, cursor* cur) {
    // if at top of file, dont do anything
    if (cur->x == 0 && cur->y == 1 && cur->column_offset == 0 && cur->line_offset == 0)
        return -1;

    // get the character thats being deleted
    char* charptr = get_char(cur, txt) - 1;

    // save it so we can return it
    char t = *charptr;

    // if we're about to delete a newline move cursor correctly
    if (t == '\n') {
        // get pointer to the line above
        char* l_above_ptr = txt->buffer;
        // move pointer to correct place
        for (int i = 1; i < cur->line_offset + cur->y - 1; i++)
            l_above_ptr += line_length(l_above_ptr) + 1;

        // move cursor to end of line above
        cur->x = line_length(l_above_ptr);
        cur->y--;

        // move column offset if needed
        if (cur->x > T_COLUMNS)
            cur->column_offset = cur->x - T_COLUMNS - 1;

        // sanity checks
        if (cur->y < 1) {
            cur->y = 1;
            cur->line_offset--;
        }
        if (cur->line_offset < 0) {
            cur->line_offset = 0;
        }
        txt->lines--;
    } else {
        // if no newline deleted, then we can just move cursor back by one
        cur->x--;
        if (cur->x < 0) {
            cur->x = 0;
            cur->column_offset--;
        }
        if (cur->column_offset < 0) {
            cur->column_offset = 0;
        }
    }

    // move everything back by one and overwrite character
    memcpy(charptr, charptr + 1, txt->length - (charptr - txt->buffer) + 1);
    txt->length--;
    return t;
}

// save the file from memory to disk
int save_file(const char* filename, const text* txt) {
    FILE* fp;
    // open the file
    if (!(fp = fopen(filename, "w"))) {
        return -1;
    }
    // puts the data
    fputs(txt->buffer, fp);

    // close the file
    fclose(fp);

    // log change
    char str[strlen(filename) + 30];
    sprintf(str, "[EDIT] Edited file %s, now has %d lines", filename, txt->lines);
    write_log(str);

    return 0;
}

// parse the command line arguments
// return 0 as an error value
// return 1 to open file (none)
// return c to copy file (-c) (filename should be given)
// return d to delete file (-d)
// return n to create file (-n)
char parse_args(int argc, char** argv) {
    switch (argc) {
        case 0:
        case 1:
            // invalid usage
            return 0;
            break;
        case 2:
            // open file as usual, only filename passed
            return 1;
            break;
        case 3:
            // two things passed, one filename one option
            if (!strcmp(argv[1], "-d"))
                return 'd';
            else if (!strcmp(argv[1], "-n"))
                return 'n';
            else
                return 0;
            break;
        case 4:
            // three things passed, one option two filenames
            if (!strcmp(argv[1], "-c"))
                return 'c';
            else
                return 0;
            break;
        default:
            return 0;
            break;
    }
}

// copy the file
// copy as binary so also works for binary files
int copy_file(const char* source, const char* target) {
    // check file doesnt already exist
    if (!access(target, F_OK)) {
        fprintf(stderr, "file already exists\n");
        return 1;
    }

    // open both files one for reading one writing
    FILE *sourcefp, *targetfp;
    if (!(sourcefp = fopen(source, "rb"))) {
        fprintf(stderr, "file not found\n");
        return -1;
    }
    if (!(targetfp = fopen(target, "wb"))) {
        fprintf(stderr, "could not create new file\n");
        return -1;
    }

    // allocate a small buffer
    unsigned char txt[50];
    int n;

    // copy 50 bytes at a time
    while ((n = fread(txt, 1, 50, sourcefp))) {
        fwrite(txt, 1, n, targetfp);
        // if less than 50 bytes was read, means eof reached so break
        if (n != 50)
            break;
    }
    // close both files
    fclose(sourcefp);
    fclose(targetfp);

    // log operation
    char str[strlen(source) + strlen(target) + 30];
    sprintf(str, "[COPY] Copied file %s to %s", source, target);
    write_log(str);

    return 0;
}

int delete_file(const char* filename) {
    if (remove(filename)) {
        fprintf(stderr, "file not found\n");
        return -1;
    }

    char str[strlen(filename) + 30];
    sprintf(str, "[DELETE] Deleted file %s", filename);
    write_log(str);

    return 0;
}

// make a blank file only in memory for opening in editor
text* create_file(const char* filename) {
    // check file doesnt already exist because we'll probably save it later
    if (!access(filename, F_OK)) {
        fprintf(stderr, "file already exists\n");
        return NULL;
    }

    // log file creation
    char str[strlen(filename) + 30];
    sprintf(str, "[CREATE] Created file %s", filename);
    write_log(str);

    // allocate a new text struct
    text* txt;
    txt = malloc(sizeof(text));
    txt->buffer = malloc(10);
    txt->bufsize = 10;
    txt->length = 0;
    txt->lines = 1;
    txt->buffer[0] = '\0';
    return txt;
}

// write an output message to the log
int write_log(const char* text) {
    // try to open file
    FILE* fp;
    if (!(fp = fopen(".kinlog", "a"))) {
        return -1;
    }
    // get timestamp
    time_t now;
    time(&now);
    // create time info struct
    struct tm* timeinfo = localtime(&now);
    char timestr[20];
    // genereate a time stamp
    strftime(timestr, 20, "%Y-%m-%d %H:%M:%S", timeinfo);
    // print to file then close it
    fprintf(fp, "[%s]%s\n", timestr, text);
    fclose(fp);
    return 0;
}