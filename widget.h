#ifndef WIDGET_H
#define WIDGET_H

#include <SDL.h>
#include "event.h"
#include "colour.h"
#include "font.h"

enum {
    signal_activate,
    signal_cancel,
    signal_lose_focus,
    signal_gain_focus,
    signal_resize,
    signal_count
};

extern SDL_Surface *screen;

struct widget_s;

typedef void (*callback_f)(struct widget_s *, void *);

struct handler_s {
    callback_f function;
    void *data;
};

typedef struct widget_s *widget_ptr;

struct widget_s {
    int (*handle_event)(widget_ptr, event_ptr);
    void (*moved)(widget_ptr);
    void (*update)(widget_ptr);
    int x, y;
    int localx, localy;
    int w, h;
    struct widget_s *parent;
    struct handler_s handler[signal_count];
    int visible;
    int can_focus;
    int has_focus;
};

typedef struct widget_s widget_t[1];

int widget_handle_event(widget_ptr w, event_ptr e);
void widget_notify_move(widget_ptr w);

void widget_init(widget_ptr);
void widget_clear(widget_ptr);
void widget_moved(widget_ptr);
void widget_update(widget_ptr w);

void widget_put_size(widget_ptr wid, int w, int h);
void widget_fillrect(widget_ptr wid, SDL_Rect *r, int c);

int in_widget(widget_ptr wid, int x, int y);
void widget_raise_signal(widget_ptr w, int sig);
void widget_fill(widget_ptr wid, int c);
void widget_blit(void *p, SDL_Surface *s, SDL_Rect *src, SDL_Rect *dst);
void widget_put_local(widget_ptr wid, int x, int y);
void widget_connect(widget_ptr w, int sig, callback_f f, void *data);
void widget_write(widget_ptr w, int x, int y, char *s);
void widget_show(widget_ptr w);
void widget_hide(widget_ptr w);

void widget_getmousexy(widget_ptr w, int *x, int *y);
int widget_has_mouse(widget_ptr wid);
int widget_getmod(widget_ptr);

//TODO: find a better home for the following:
void update_mousestate();
void update_modstate();

void widget_focus(widget_ptr);

void widget_line(widget_ptr w, int x1, int y1, int x2, int y2, int c);
void widget_circle(widget_ptr w, int x, int y, int r, int c);
void widget_filled_circle(widget_ptr w, int x, int y, int r, int c);
void widget_filled_polygon(widget_ptr w, int *x, int *y, int n, int c);
void widget_rectangle(widget_ptr w, int x1, int y1, int x2, int y2, int c);
SDL_Surface *new_image(int w, int h);

void widget_lose_focus(widget_ptr w);
#endif //WIDGET_H
