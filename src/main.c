#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "filehandler.h"
#include "gui.h"

/*
 macro for control keys
 defined as a macro so it is a compile-time constant and can be used in a switch statement
 C constants are not compile-time const, and the C language does not support C++'s constexpr functions yet,
 which would allow this to be declared as a real function instead, and would be much nicer.
 */
#define CTRL(x) ((x)&0x1f)

// the entry point of the program
int main(int argc, char* argv[]) {
    // handle command line args
    // pass argc/v to parsing function
    char opt = parse_args(argc, argv);

    // if error value is returned, give invalid usage message and exit
    if (!opt) {
        fprintf(stderr, "Invalid usage! See documentation on proper usage.\n");
        return -1;
    }

    // store a pointer to the filename
    char* filename;
    if (opt == 1)
        filename = argv[1];
    else
        filename = argv[2];

    text* filetext;
    // perform the action required by any special flags
    if (opt == 'c') {
        return copy_file(filename, argv[3]);
    } else if (opt == 'd') {
        return delete_file(filename);
    } else if (opt == 'n') {
        filetext = create_file(filename);
        if (!filetext) {
            return -1;
        }
    }

    // finished messing with command line args, down to business
    // declare the filetext struct containing the file's text along with other relevant info about it
    // then fill it with data by reading the file
    // dont try to open a file if its a new file as there is no file to open
    if (opt != 'n' && !(filetext = read_file(filename))) {
        // this check has already been made so something must really go wrong here
        fprintf(stderr, "Error, cannot open file.\n");
        return -1;
    }

    // delcare windows
    WINDOW* textview;  // the main window where text goes
    WINDOW* output;    // the ouput console on the left
    WINDOW* info;      // the box with commands

    // initalise ncurses stuff, including the 3 windows that make up the display
    init_gui(&textview, &output, &info);

    // initalise and define the cursors and its initial position
    cursor cursor = {0, 1, 0, 0};

    // say a nice hello
    write_ouput(output, "Welcome to kin!");

    // draw intitial text
    view_text(textview, filetext, &cursor);
    draw_info(info);
    draw_border(textview, filename);
    // set the terminal cursor in the right place
    wmove(textview, cursor.y, cursor.x + 1);
    // refresh
    wrefresh(textview);

    // the variable where each keypress is stored
    // int not char because there are more than 127 buttons/key combos on a keyboard
    int chr;
    // the top of this loop is a blocking input
    // meaning the program only does something when an input is received
    // this makes everything nice and easy to handle

    while ((chr = wgetch(textview))) {
        switch (chr) {
            // arrow key input, simply move cursor by relevant amount
            case KEY_LEFT:
                move_cursor(&cursor, filetext, -1, 0);
                break;
            case KEY_RIGHT:
                move_cursor(&cursor, filetext, 1, 0);
                break;
            case KEY_UP:
                move_cursor(&cursor, filetext, 0, -1);
                break;
            case KEY_DOWN:
                move_cursor(&cursor, filetext, 0, 1);
                break;
            // save and exit
            case CTRL('x'):
                if (save_file(filename, filetext))
                    write_ouput(output, "Error saving file");
                else {
                    write_ouput(output, "File saved.");
                    // wait a while so user can see whats going on
                    // (perhaps toooo much delay tho ? mayb fix)
                    sleep(1);
                    write_ouput(output, "Exiting...");
                    sleep(1);
                    endwin();
                    return -1;
                }
                break;
            // save no exit
            case CTRL('s'):
                if (save_file(filename, filetext))
                    write_ouput(output, "Error saving file");
                else
                    write_ouput(output, "File saved.");
                break;
            // exit no save
            case CTRL('c'):
                write_ouput(output, "Exiting...");
                sleep(1);
                endwin();
                return -1;
                break;
            // give line count
            // have to define a new scope with braces because we're declaring new variables
            case CTRL('n'): {
                char str[30];
                sprintf(str, "File contains %d lines", filetext->lines);
                write_ouput(output, str);
                break;
            }
            // word and char count
            case CTRL('w'): {
                char str[50];

                sprintf(str, "File contains %d words and %ld charachters", words(filetext->buffer), strlen(filetext->buffer));
                write_ouput(output, str);
                break;
            }
            default:
                break;
        }
        // more complex condition cannot be handled by case statement
        // charachters to insert
        // anything space and upwards
        if (chr >= 32 && chr <= 126) {
            ins_char(filetext, &cursor, chr);
        }
        // 4 spaces because thats what tabs are, right?
        if (chr == '\t') {
            ins_char(filetext, &cursor, ' ');
            ins_char(filetext, &cursor, ' ');
            ins_char(filetext, &cursor, ' ');
            ins_char(filetext, &cursor, ' ');
        }
        // insert newline
        else if (chr == '\n') {
            ins_newline(filetext, &cursor);
        }
        // remove a charachter
        else if (chr == KEY_BACKSPACE) {
            del_char(filetext, &cursor);
        }
        // charachter insertion/deletion needs the cursor so that in can move it appropriate to where the charachter was inserted or removed respective to the line it's on.

        // erase old text and draw new
        werase(textview);
        view_text(textview, filetext, &cursor);
        draw_border(textview, filename);
        wmove(textview, cursor.y, cursor.x + 1);
        wrefresh(textview);
    }

    // this should never be reached but its here anyway just in case
    endwin();
    return 0;
}