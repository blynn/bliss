#ifndef WIDGET_H
#define WIDGET_H

#include <SDL.h>
#include <string.h>

#include "util.h"
#include "darray.h"
#include "colour.h"

enum {
    code_interrupt,
    code_motion,
};

typedef SDL_Rect rect_t[1];
typedef SDL_Rect *rect_ptr;

struct widget_s;
typedef struct widget_s widget_t[1];
typedef struct widget_s *widget_ptr;
struct widget_s {
    int localx, localy;
    int w, h;
    int globalx, globaly;
    widget_ptr parent;
    darray_t show_list;
    darray_t child;
    void (*handle_mousebuttondown)(widget_ptr w, int button, int x, int y);
    void (*update)(widget_ptr w);
    void (*put_size)(widget_ptr w, int x, int y);
};

typedef SDL_Surface image_t[1];
typedef SDL_Surface *image_ptr;
typedef SDL_Surface image_s;

void enable_key_repeat();
void disable_key_repeat();

image_ptr image_new(int w, int h);
void image_free(image_ptr img);
void image_box_rect(image_ptr img, int c);
void image_string(image_ptr p, int x, int y, char *s, int c);
void image_blit_from_screen(image_ptr img, rect_ptr r);

void image_filled_circle(image_ptr img, int x, int y, int r, int c);

void widget_set_screen(SDL_Surface *s);
int local_contains(widget_ptr r, int x, int y);
int global_contains(widget_ptr r, int x, int y);
int widget_contains(widget_ptr w, int x, int y);

void widget_pixel(widget_ptr r, int x, int y, int c);
void widget_line(widget_ptr r, int x0, int y0, int x1, int y1, int c);
void widget_filled_trigon(widget_ptr w,
	int x0, int y0,
	int x1, int y1,
	int x2, int y2,
	int c);
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
void widget_draw_children(widget_ptr p);
//void widget_add_child(widget_ptr w, widget_ptr child);
void widget_hide(widget_ptr w);
void widget_show(widget_ptr w);
void widget_init(widget_ptr w, widget_ptr parent);
void widget_clear(widget_ptr w);

void widget_put_size(widget_ptr w, int x, int y);

void widget_filled_circle(widget_ptr w, int x, int y, int r, int c);
void widget_circle(widget_ptr w, int x, int y, int r, int c);

void widget_put_geometry(widget_ptr wid, int x, int y, int w, int h);
void widget_put_location(widget_ptr wid, int x, int y);

static inline void widget_update(widget_ptr w)
{
    w->update(w);
}

void append_request(int x, int y, int w, int h);
static inline void request_update(widget_ptr w)
{
    append_request(w->globalx, w->globaly, w->w, w->h);
}

static inline void request_update_rect(widget_ptr w, rect_ptr r)
{
    append_request(r->x + w->globalx, r->y + w->globaly, r->w, r->h);
}
void process_request();

typedef void (*motioncb)(widget_ptr, int x0, int y0, int x1, int y1, void *);
typedef int (*keydowncb)(widget_ptr w, int sym, int mod, void *data);

struct widget_callback_s {
    widget_ptr w;
    void *func;
    void *data;
};

void widget_system_init();
typedef struct widget_callback_s widget_callback_t[1];
typedef struct widget_callback_s *widget_callback_ptr;
void widget_bind_mouse_motion(widget_ptr w, motioncb func, void *data);
void widget_unbind_mouse_motion(widget_ptr w);

typedef int (*buttoncb)(widget_ptr, int button, int x, int y, void *);
void widget_on_next_button_up(widget_ptr w, buttoncb func, void *data);

void root_mouse_motion(int x0, int y0, int x1, int y1);
void root_button_up(widget_ptr w, int button, int x, int y);
void root_button_down(widget_ptr w, int button, int x, int y);
void root_key_down(int sym, int mod);
void widget_push_keydowncb(widget_ptr w, keydowncb func, void *data);
void widget_pop_keydowncb();
void widget_push_buttondowncb(widget_ptr w, buttoncb func, void *data);
void widget_pop_buttondowncb();
void motion_notify();
void widget_hide_all(widget_ptr w);
void screen_blit(image_ptr img, rect_ptr imgr, rect_ptr screenr);
void screen_capture(rect_ptr screenr, image_ptr img);

#endif //WIDGET_H
