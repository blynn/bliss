#include <stdlib.h>
#include "window.h"

static void window_compute_geometry(window_ptr win)
{
    int x, y;
    int w, h;

    x = y = 0;
    w = ((widget_ptr) win)->w;
    h = ((widget_ptr) win)->h;
    if (win->has_border) {
	x += window_border;
	y += window_border;
	w -= 2 * window_border;
	h -= 2 * window_border;
    }
    if (win->has_titlebar) {
	y += window_title_h;
	h -= window_title_h;
    }
    widget_put_local((widget_ptr) win->con, x, y);
    widget_put_size((widget_ptr) win->con, w, h);
}

void window_moved(widget_ptr w, void *data)
{
    window_ptr win = (window_ptr) w;
    container_ptr con = win->con;
    widget_notify_move((widget_ptr) con);
    window_compute_geometry(win);
}

void window_focus(window_ptr win, widget_ptr w)
{
    if (win->focus_widget) widget_lose_focus(win->focus_widget);
    if (w) {
	if (w->can_focus) {
	    win->focus_widget = w;
	    w->has_focus = 1;
	} else {
	    win->focus_widget = NULL;
	}
    } else {
	win->focus_widget = NULL;
    }
}

static void window_handle_click(window_ptr win, event_ptr event)
{
    int i, n;
    container_ptr c = win->con;
    widget_ptr w = (widget_ptr) c;

    //focus widget must be handled separately
    //it might not be a child of the window (TODO: fix this?)
    if (win->focus_widget) {
	if (widget_has_mouse(win->focus_widget)) {
	    widget_handle_event(win->focus_widget, event);
	    return;
	}
    }
    n = c->child->count;
    for(i=0; i<n; i++) {
	widget_ptr wi = (widget_ptr) c->child->item[i];
	if (widget_has_mouse(wi)) {
	    widget_focus(wi);

	    widget_handle_event(wi, event);
	    return;
	}
    }
    widget_focus(NULL);
    widget_handle_event(w, event);
}

static void window_handle_key(window_ptr win, event_ptr event)
//widget with focus gets the keystrokes
{
    widget_ptr w = (widget_ptr) win->con;
    if (win->focus_widget) {
	widget_handle_event(win->focus_widget, event);
    } else {
	if (win->handle_key) {
	    if (win->handle_key((widget_ptr) win, event)) {
		widget_focus(NULL);
		return;
	    }
	}
	widget_handle_event(w, event);
    }
}

int window_handle_event(widget_ptr w, event_ptr e)
{
    window_ptr win = (window_ptr) w;
    container_ptr con = win->con;

    switch(e->type) {
	case SDL_MOUSEBUTTONDOWN:
	    window_handle_click(win, e);
	    break;
	case SDL_KEYDOWN:
	    window_handle_key(win, e);
	    break;
	default:
	    widget_handle_event((widget_ptr) con, e);
	    break;
    }
    return 1;
}

void window_update(widget_ptr w)
{
    SDL_Rect r;

    window_ptr win = (window_ptr) w;
    container_ptr con = win->con;

    widget_fill(w, c_background);
    if (win->has_border) {
	widget_draw_border(w);
    }
    if (win->has_titlebar) {
	r.x = 3;
	r.y = 3;
	r.w = w->w - 6;
	r.h = window_title_h;
	widget_fillrect(w, &r, c_titlebar);
	r.x = 10;
	r.y = 3;
	if (win->titleimage) widget_blit(w, win->titleimage, NULL, &r);
    }

    widget_update((widget_ptr) con);
    if (win->focus_widget) {
	widget_update(win->focus_widget);
    }
}

void window_put_widget(window_ptr win, widget_ptr w, int x, int y)
{
    container_ptr con = win->con;
    container_put_widget(con, w, x, y);
}

void window_set_style(window_ptr win, int border, int titlebar)
{
    win->has_border = border;
    win->has_titlebar = titlebar;
    window_compute_geometry(win);
}

void window_init(window_ptr win)
{
    widget_ptr w = (widget_ptr) win;
    container_ptr con = win->con;

    widget_init(w);
    container_init(con);
    ((widget_ptr) con)->parent = w;
    widget_connect(w, signal_move, window_moved, NULL);
    w->update = window_update;
    w->handle_event = window_handle_event;
    win->focus_widget = NULL;
    win->handle_key = NULL;
    window_set_style(win, 1, 1);
    win->titleimage = NULL;

    main_add_window(win);
}

void window_put_title(window_ptr win, char *s)
{
    if (win->titleimage) SDL_FreeSurface(win->titleimage);
    if (s) win->titleimage = font_rendertext(s);
}

void window_clear(window_ptr win)
{
    widget_ptr w = (widget_ptr) win;
    container_ptr con = win->con;
    widget_clear(w);
    container_clear(con);
}

window_ptr window_new()
{
    window_ptr w = (window_ptr) malloc(sizeof(window_t));

    window_init(w);
    return w;
}

int window_titlebar_has_mouse(window_ptr win)
{
    widget_ptr w = (widget_ptr) win;
    int x, y;
    int x0, x1, y0, y1;

    if (!win->has_titlebar) return 0;

    x0 = 0;
    y0 = 0;
    x1 = w->w - 1;
    y1 = window_title_h - 1;
    if (win->has_border) {
	x0++;
	x1--;
	y0++;
	y1++;
    }

    widget_getmousexy((widget_ptr) win, &x, &y);
    if (x < x0 || x > x1 || y < y0 || y > y1) return 0;
    return 1;
}

void window_modal_open(window_ptr win)
{
    main_open_modal_window(win);
}

void window_close(window_ptr win)
{
    main_close_window(win);
}
