#ifndef WINDOW_H
#define WINDOW_H

#include "widget.h"

struct window_s;
typedef struct window_s window_t[1];
typedef struct window_s *window_ptr;

struct window_s {
    widget_t w;
    char *title;
    widget_t body;
};

void window_init(window_ptr win, widget_ptr parent);

#endif //WINDOW_H
