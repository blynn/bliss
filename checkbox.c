#include <SDL.h>
#include <stdlib.h>
#include "checkbox.h"

static image_ptr checkbox_image()
{
    static image_ptr img = NULL;
    if (!img) {
	img = SDL_LoadBMP("tick.bmp");
    }
    return img;
}

void checkbox_update(checkbox_ptr b)
{
    widget_ptr w = b->w;
    widget_lowered_border(w);
    if (b->state) widget_blit(w, 2, 2, checkbox_image());
    else widget_box(w, 2, 2, w->w - 3, w->h - 3, c_textboxbg);
}

void checkbox_handle_mousebuttondown(widget_ptr w, int checkbox, int x, int y)
{
    checkbox_ptr b = (checkbox_ptr) w;

    b->state = !b->state;

    w->update(w);
}

void checkbox_init(checkbox_ptr b, widget_ptr parent)
{
    widget_init(b->w, parent);
    b->w->update = (void (*)(widget_ptr)) checkbox_update;
    b->w->handle_mousebuttondown = checkbox_handle_mousebuttondown;
    b->w->w = 12;
    b->w->h = 12;
}

checkbox_ptr checkbox_new(widget_ptr parent)
{
    checkbox_ptr res = (checkbox_ptr) malloc(sizeof(checkbox_t));
    checkbox_init(res, parent);
    return res;
}
