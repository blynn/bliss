#ifndef WINDOW_H
#define WINDOW_H

#include "widget.h"
#include "container.h"

enum {
    window_border = 2,
    window_title_h = 16,
};

struct window_s;
typedef struct window_s *window_ptr;
typedef struct window_s window_t[1];

struct window_s {
    widget_t w;
    container_t con;
    widget_ptr focus_widget;
    int has_border;
    int has_titlebar;
    SDL_Surface *titleimage;
    //TODO: remove this kludge:
    int (* handle_key)(widget_ptr w, event_ptr e);
};

void window_init(window_ptr);
void window_clear(window_ptr);
void window_put_widget(window_ptr win, widget_ptr w, int x, int y);

void window_set_style(window_ptr win, int border, int titlebar);

window_ptr window_new();
int window_titlebar_has_mouse(window_ptr win);
void window_close(window_ptr win);
void window_put_title(window_ptr win, char *s);
void window_modal_open(window_ptr win);
void window_focus(window_ptr win, widget_ptr w);

#endif //WINDOW_H
