#include "window.h"

void win_handle_mousebuttondown(widget_ptr w, int button, int x, int y)
{
    int i;
    for (i=0; i<w->show_list->count; i++) {
	widget_ptr p = w->show_list->item[i];
	if (local_contains(p, x, y)) {
	    p->handle_mousebuttondown(p, button,
		    x - p->localx, y - p->localy);
	    return; 
	}
    }
}

static int window_button_down(widget_ptr w, int button, int x, int y, void *data)
{
    if (widget_contains(w, x, y)) {
	win_handle_mousebuttondown(w, button, x, y);
	return 0;
    }
    return 0;
}

void window_open(window_ptr w)
{
    widget_push_buttondowncb(w->w, window_button_down, NULL);
    widget_show(w->w);
    widget_update(w->w);
    request_update(w->w);
}

void window_close(window_ptr w)
{
    widget_pop_buttondowncb();
    widget_hide(w->w);
    widget_update(w->w->parent);
    request_update(w->w->parent);
}

void window_update(widget_ptr w)
{
    window_ptr win = (window_ptr) w;

    w = win->w;
    widget_raised_background(w);
    widget_box(w, 2, 2, w->w - 3, 20, c_darkshadow);
    widget_string(w, 8, 8, win->title, c_highlight);

    win->body->update(win->body);
}

void win_put_size(widget_ptr w, int x, int y)
{
    window_ptr win = (window_ptr) w;

    widget_put_size(w, x, y);
    win->body->w = x - 4;
    win->body->h = y - 2 - 16;
}

void window_init(window_ptr win, widget_ptr parent)
{
    widget_init(win->w, parent);
    win->w->handle_mousebuttondown = win_handle_mousebuttondown;
    win->w->update = window_update;
    win->w->put_size = win_put_size;
    widget_init(win->body, win->w);
    win->body->localx = 2;
    win->body->localy = 2 + 16;
    widget_show(win->body);
}
