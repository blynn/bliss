#ifndef GRID_H
#define GRID_H

#include "pattern.h"
#include "widget.h"
#include "textbox.h"

enum {
    flag_hide_row_borders = 1,
};

struct grid_s;
typedef struct grid_s *grid_ptr;
typedef struct grid_s grid_t[1];

struct grid_s {
    widget_t widget;
    int cr, cc; //cursor
    int or, oc; //offset
    int rmax, cmax;
    int xmax, ymax;
    char *(*row_label)(grid_ptr, int);
    char *(*col_label)(grid_ptr, int);
    char *(*at)(grid_ptr, int row, int col);
    void (*put)(grid_ptr, char *text, int row, int col);
    int flag;
    int is_edit;
    textbox_t editbox;
};

void grid_init(grid_ptr);
void grid_clear(grid_ptr);

grid_ptr grid_new();

#endif //GRID_H
