#include <stdlib.h>
#include "listbox.h"

void listbox_update(widget_ptr w)
{
    listbox_ptr b = (listbox_ptr) w;
    SDL_Rect rect;
    widget_rectangle(w, 0, 0, w->w - 1, w->h - 1, c_border);
    rect.x = 3;
    rect.y = 2;
    if (b->image) {
	widget_blit(w, b->image, NULL, &rect);
    }
}

int listbox_handle_event(widget_ptr w, event_ptr e)
{
    return 0;
}

void listbox_init(listbox_ptr b)
{
    widget_ptr w = (widget_ptr) b;
    widget_init(w);
    w->update = listbox_update;
    w->handle_event = listbox_handle_event;
    b->image = NULL;
}

listbox_ptr listbox_new()
{
    listbox_ptr b;
    b = (listbox_ptr) malloc(sizeof(struct listbox_s));
    listbox_init(b);
    return b;
}

void listbox_put_text(listbox_ptr b, char *s)
{
    if (b->image) SDL_FreeSurface(b->image);
    if (s) b->image = font_rendertext(s);
}

void listbox_clear(listbox_ptr b)
{
    widget_ptr w = (widget_ptr) b;
    if (b->image) SDL_FreeSurface(b->image);
    widget_clear(w);
}
