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
    void (*insert)(grid_ptr, int row, int col);
    void (*delete)(grid_ptr, int row, int col);
    void (*row_click)(grid_ptr, int row);
    int flag;
    int is_edit;
    textbox_t editbox;
};

void grid_init(grid_ptr);
void grid_clear(grid_ptr);

grid_ptr grid_new();

void grid_draw_hline(grid_ptr g, int y, int p, int q);

void grid_update(widget_ptr w);

int grid_handle_event(widget_ptr w, event_ptr e);

#endif //GRID_H
