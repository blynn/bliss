#include <stdlib.h>
#include "label.h"

void label_shrinkwrap(label_ptr l)
{
    widget_ptr w = (widget_ptr) l;
    widget_put_size(w, l->image->w + 6, l->image->h + 6);
}

void label_update(widget_ptr w)
{
    SDL_Rect rect;
    label_ptr b = (label_ptr) w;
    rect.x = 3;
    rect.y = 3;
    if (b->image) {
	widget_blit(w, b->image, NULL, &rect);
    }
}

int label_handle_event(widget_ptr w, event_ptr e)
{
    return 0;
}

void label_init(label_ptr b)
{
    widget_ptr w = (widget_ptr) b;
    widget_init(w);
    w->update = label_update;
    w->handle_event = label_handle_event;
    b->image = NULL;
}

label_ptr label_new()
{
    label_ptr b;
    b = (label_ptr) malloc(sizeof(struct label_s));
    label_init(b);
    return b;
}

void label_put_text(label_ptr b, char *s)
{
    if (b->image) SDL_FreeSurface(b->image);
    if (s) b->image = font_rendertext(s);
}

void label_clear(label_ptr b)
{
    widget_ptr w = (widget_ptr) b;
    if (b->image) SDL_FreeSurface(b->image);
    widget_clear(w);
}
