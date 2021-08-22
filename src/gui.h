#ifndef GUI_H
#define GUI_H
#include <ncurses.h>

#include "common.h"

// size of the screen
#define WIDTH 120
#define HEIGHT 36

void write_ouput(WINDOW* win, const char* str);

void draw_border(WINDOW* win, const char* filename);

int init_gui(WINDOW** w1, WINDOW** w2, WINDOW** w3);

void draw_info(WINDOW* win);

void view_text(WINDOW* win, const text* txt, const cursor* cur);

void move_cursor(cursor* cur, text* txt, int dx, int dy);

#endif
