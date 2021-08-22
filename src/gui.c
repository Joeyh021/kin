#include "gui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// function to write a message to the output console on the screen
void write_ouput(WINDOW* win, const char* str) {
    // hide the cursor while we do this
    curs_set(0);

    // 78 is width of output box
    // 75 max length can be written
    // copy the old messages all up one line
    // copying charachters from screen so use curses special chtype
    chtype ch;
    for (int y = 2; y <= 6; y++) {
        for (int x = 2; x < 79; x++) {
            // read it from the screen
            ch = mvwinch(win, y, x);

            // put it back but up by one space
            mvwaddch(win, y - 1, x, ch & A_CHARTEXT);
        }
    }

    // overwrite bottom row with all spaces so its clear
    for (int i = 2; i < 79; i++)
        mvwaddch(win, 6, i, ' ');

    // add the nice prompt
    mvwaddstr(win, 6, 2, "> ");

    // add our string
    mvwaddnstr(win, 6, 4, str, 75);

    // refresh
    wrefresh(win);

    // show cursor again
    curs_set(1);
}

// the myriad of curses initialisation stuff that needs to happen
int init_gui(WINDOW** w1, WINDOW** w2, WINDOW** w3) {
    // puts terminal into curses mode
    initscr();

    // get all keyboard input as-is
    raw();

    // make input not echo immediately to screen
    noecho();

    // get size of the screen
    int max_rows, max_cols;
    getmaxyx(stdscr, max_cols, max_rows);
    // if not big enough, exit.
    if ((max_rows < WIDTH) || (max_cols < HEIGHT)) {
        mvprintw(0, 0, "screen too small! %dx%d minimum size required.", WIDTH, HEIGHT);
        mvprintw(1, 0, "press any key to exit...");
        refresh();
        if (getch()) {
            endwin();
            return -1;
        }
    }

    // initalise our three windows and set pointers to them
    // sizes are chosen by design
    // height, width, Vpos, Hpos

    // create the window
    *w1 = newwin(28, 118, 0, 1);
    // draw a nice box around it
    box(*w1, 0, 0);
    // refresh it
    wrefresh(*w1);
    // only for the text one - allow full keyboard input
    keypad(*w1, TRUE);

    *w2 = newwin(8, 80, 28, 1);
    box(*w2, 0, 0);
    wrefresh(*w2);

    *w3 = newwin(8, 37, 28, 82);
    box(*w3, 0, 0);
    wrefresh(*w3);

    return 0;
}

// draws the nice little info box
void draw_info(WINDOW* win) {
    // just prints a bunch of strings
    mvwaddstr(win, 0, 1, "COMMANDS");
    mvwaddstr(win, 1, 1, "CTRL + S - save the file to disk");
    mvwaddstr(win, 2, 1, "CTRL + C - exit without saving");
    mvwaddstr(win, 3, 1, "CTRL + X - save and exit");
    mvwaddstr(win, 4, 1, "CTRL + N - show line count");
    mvwaddstr(win, 5, 1, "CTRL + W - show word count");
    // and then refreshes
    wrefresh(win);
}

// takes the character array of text and draws it to screen in lines
void view_text(WINDOW* win, const text* text, const cursor* cur) {
    // set the line pointer to the top of the first line
    char* lineptr = text->buffer;

    // skip ahead by line offset to the first line that we want to render
    for (int i = 0; i < cur->line_offset; i++)
        lineptr += line_length(lineptr) + 1;

    int len = 0;
    // for every line in the box
    for (int line = 1; line <= T_LINES; line++) {
        // if no more lines available to draw, stop.
        if (cur->line_offset + line > text->lines)
            break;

        // the length of the line about to be drawn
        len = line_length(lineptr);

        // if line is not out of view (ie column offset is greater than its length)
        if (len - cur->column_offset > 0) {
            // jump to start of line
            char* chptr = lineptr + cur->column_offset;

            // print each letter of it
            for (int c = 0; c < T_COLUMNS; c++) {
                // if end of line, break
                if (chptr[c] == '\n' || chptr[c] == '\0')
                    break;

                // add char at a time
                mvwaddch(win, line, c + 1, chptr[c]);
            }
        }
        // jump to next line
        lineptr += len + 1;
    }
}

// draw the main text window border
// includes the file name for handy reference
void draw_border(WINDOW* win, const char* filename) {
    // draw box
    box(win, 0, 0);
    // draw our name
    mvwaddstr(win, 0, 1, "KIN - ");
    // draw file name
    mvwaddnstr(win, 0, 7, filename, T_COLUMNS - 6);
}

// the cursor moving function
// takes the cursor and text info aswell as x and y offsets as args
void move_cursor(cursor* cur, text* txt, int dx, int dy) {
    // a pointer to the start of the line above the one the cursor is currently on
    char* l_above_ptr = txt->buffer;
    // iterate til we get there
    for (int i = 1; i < cur->line_offset + cur->y - 1; i++)
        l_above_ptr += line_length(l_above_ptr) + 1;

    // the current line pointer
    char* l_current_ptr;

    // if there is no line above (ie we're on top line) null the line above pointer
    if (cur->line_offset == 0 && cur->y == 1) {
        l_current_ptr = l_above_ptr + 1;
        l_above_ptr = NULL;
    }
    // otherwise, set the current line pointer using the line_length function and some pointer arithmetic
    else
        l_current_ptr = l_above_ptr + line_length(l_above_ptr) + 1;

    if (dy == 1) {
        // if on bottom line, cant move
        if (cur->line_offset + cur->y >= txt->lines)
            return;

        // get some line lengths
        int l_current_len = line_length(l_current_ptr);
        int l_below_len = line_length(l_current_ptr + l_current_len + 1);

        // if the line below is shorter than the line we're on
        // then move down, but move to the end of the line below
        // so we arent moving into empty space
        if (l_below_len < cur->x + cur->column_offset) {
            cur->x = l_below_len;

            // account for the column offset too
            if (cur->column_offset > 0) {
                cur->column_offset = cur->x - T_COLUMNS - 1;
            }
        }
        cur->y++;
    }

    else if (dy == -1) {
        // if on top line, cant move
        if (cur->line_offset == 0 && cur->y == 1)
            return;

        int l_above_len = line_length(l_above_ptr);

        // same as before, except when moving up instead of down
        if (l_above_len < cur->x + cur->column_offset) {
            cur->x = l_above_len;
            if (cur->column_offset > 0) {
                cur->column_offset = cur->x - T_COLUMNS - 1;
            }
        }
        cur->y--;
    }

    else if (dx == 1) {
        // if at end of file, cant move right
        if (*get_char(cur, txt) == '\0')
            return;
        // if at end of line, move to start of next line, accounting for offset too
        if (*get_char(cur, txt) == '\n') {
            cur->x = 0;
            cur->column_offset = 0;
            cur->y++;
        } else {
            cur->x++;
        }
    }

    else if (dx == -1) {
        // if at top of file cant move left
        if (cur->column_offset == 0 && cur->line_offset == 0 && cur->x == 0 && cur->y == 1)
            return;

        // if at start of line, moving left means move to end of line above
        if (*(get_char(cur, txt) - 1) == '\n') {
            cur->x = line_length(l_above_ptr);
            // again account for offset
            if (cur->x > T_COLUMNS)
                cur->column_offset = cur->x - T_COLUMNS - 1;
            cur->y--;
        } else {
            cur->x--;
        }
    }

    // sanity checks and accounts for moving offsets around
    if (cur->x < 0) {
        cur->x = 0;
        cur->column_offset--;

    } else if (cur->x > T_COLUMNS) {
        cur->x = T_COLUMNS;
        cur->column_offset++;
    }

    if (cur->y < 1) {
        cur->y = 1;
        cur->line_offset--;
    } else if (cur->y > T_LINES) {
        cur->y = T_LINES;
        cur->line_offset++;
    }

    if (cur->line_offset < 0) {
        cur->line_offset = 0;
    }

    if (cur->column_offset < 0) {
        cur->column_offset = 0;
    }
    if (cur->line_offset + cur->y > txt->lines) {
        cur->y--;
    }
}
