#include <stdlib.h>
#include "button.h"

void button_shrinkwrap(button_ptr b)
{
    widget_ptr w = (widget_ptr) b;
    widget_put_size(w, b->image->w + 2, b->image->h + 2);
}

void button_update(widget_ptr w)
{
    button_ptr b = (button_ptr) w;
    SDL_Rect rect;
    rect.x = 1;
    rect.y = 1;
    widget_rectangle(w, 0, 0, w->w - 1, w->h - 1, c_border);
    if (b->image) {
	widget_blit(w, b->image, NULL, &rect);
    }
}

int button_handle_event(widget_ptr w, event_ptr e)
{
    switch (e->type) {
	case SDL_MOUSEBUTTONDOWN:
	    if (e->button.button == SDL_BUTTON_LEFT) {
		if (widget_has_mouse(w)) {
		    widget_raise_signal(w, signal_activate);
		    return 1;
		}
	    }
	    break;
	default:
	    break;
    }
    return 0;
}

void button_init(button_ptr b)
{
    widget_ptr w = (widget_ptr) b;
    widget_init(w);
    w->update = button_update;
    w->handle_event = button_handle_event;
    b->image = NULL;
}

button_ptr button_new()
{
    button_ptr b;
    b = (button_ptr) malloc(sizeof(struct button_s));
    button_init(b);
    return b;
}

void button_put_image(button_ptr b, SDL_Surface *img)
{
    b->image = img;
}

void button_clear(button_ptr b)
{
    widget_ptr w = (widget_ptr) b;
    widget_clear(w);
}
