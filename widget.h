#ifndef WIDGET_H
#define WIDGET_H

#include <SDL.h>
#include "darray.h"
#include "colour.h"

enum {
    state_normal,
    state_textbox,
    state_button_pushed,
    state_drag_vertex,
    state_drag_port,
    state_menu,
    state_quit
};

extern int state;

typedef SDL_Rect rect_t[1];

struct widget_s;
typedef struct widget_s widget_t[1];
typedef struct widget_s *widget_ptr;
struct widget_s {
    int localx, localy;
    int w, h;
    int x, y;
    widget_ptr parent;
    darray_t show_list;
    darray_t child;
    void (*handle_mousebuttondown)(widget_ptr w, int button, int x, int y);
    void (*handle_mousebuttonup)(widget_ptr w, int button, int x, int y);
    void (*handle_keydown)(widget_ptr w, int sym, int mod);
    void (*update)(widget_ptr w);
    void (*put_size)(widget_ptr w, int x, int y);
};

typedef SDL_Surface image_t[1];
typedef SDL_Surface *image_ptr;
typedef SDL_Surface image_s;

image_ptr image_new(int w, int h);
void image_clear(image_ptr img);
void image_box_rect(image_ptr img, int c);
void image_string(image_ptr p, int x, int y, char *s, int c);

void widget_set_screen(SDL_Surface *s);
int widget_contains(widget_ptr r, int x, int y);
void widget_shift_rect(widget_ptr r, widget_ptr shift);
void widget_line(widget_ptr r, int x0, int y0, int x1, int y1, int c);
void widget_box(widget_ptr r, int x0, int y0, int x1, int y1, int c);
void widget_box_rect(widget_ptr r, int c);
void widget_rectangle(widget_ptr r, int x0, int y0, int x1, int y1, int c);
void widget_rectangle_rect(widget_ptr r, int c);
void widget_string(widget_ptr r, int x, int y, char *s, int c);
void widget_raised_border(widget_ptr rect);
void widget_lowered_border(widget_ptr rect);
void widget_raised_background(widget_ptr rect);
void widget_blit(widget_ptr rect, int x, int y, image_ptr img);
void widget_translate(widget_ptr wid, int x, int y);
void widget_clip(widget_ptr wid);
void widget_unclip();
void widget_move_children(widget_ptr p);
//void widget_add_child(widget_ptr w, widget_ptr child);
void widget_hide(widget_ptr w);
void widget_show(widget_ptr w);
void widget_init(widget_ptr w, widget_ptr parent);

void widget_put_size(widget_ptr w, int x, int y);

void widget_filled_circle(widget_ptr w, int x, int y, int r, int c);

#endif //WIDGET_H
