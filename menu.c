#include <stdlib.h>
#include "menu.h"
#include "util.h"

enum {
    menuitem_pad = 5,
    menu_border = 1,
};

void menuitem_clear(menuitem_ptr m)
{
    if (m->img) SDL_FreeSurface(m->img);
    widget_clear((widget_ptr) m);
}

void menuitem_update(widget_ptr w)
{
    SDL_Rect dst;
    menuitem_ptr it = (menuitem_ptr) w;

    if (widget_has_mouse(w)) widget_fill(w, c_menubghi);
    else widget_fill(w, c_menubg);

    dst.x = menuitem_pad;
    dst.y = 1;
    widget_blit(w, it->img, NULL, &dst);
}

void menuitem_put_text(menuitem_ptr m, char *s)
{
    widget_ptr w = (widget_ptr) m;
    if (m->text) free(m->text);
    m->text = strclone(s);
    if (m->img) SDL_FreeSurface(m->img);
    m->img = font_rendertext(s);
    widget_put_size(w, m->img->w + 2 * menuitem_pad, menubar_h);
}

int menuitem_handle_event(widget_ptr w, event_ptr e)
{
    menuitem_ptr it = (menuitem_ptr) w;

    switch (e->type) {
	case SDL_MOUSEBUTTONDOWN:
	    if (widget_has_mouse((widget_ptr) it)) {
		widget_focus(NULL);
		widget_raise_signal((widget_ptr) it, signal_activate);
		return 1;
	    }
	    break;
    }
    return 0;
}

static void open_menu(widget_ptr it, void *data)
{
    widget_ptr w = (widget_ptr) data;
    widget_put_local(w, it->x, it->y + it->h);
    menu_popup((menu_ptr) w);
}

void menuitem_set_submenu(menuitem_ptr it, menu_ptr m)
{
    widget_connect((widget_ptr) it, signal_activate, open_menu, m);
}

void menuitem_init(menuitem_ptr m)
{
    widget_ptr w = (widget_ptr) m;
    widget_init(w);
    w->update = menuitem_update;
    w->handle_event = menuitem_handle_event;
    m->text = NULL;
    m->img = NULL;
}

menuitem_ptr menuitem_new()
{
    menuitem_ptr it;

    it = (menuitem_ptr) malloc(sizeof(menuitem_t));
    menuitem_init(it);

    return it;
}

int menubar_handle_event(widget_ptr p, event_ptr e)
{
    int i, n;
    menubar_ptr m = (menubar_ptr) p;
    menuitem_ptr it;

    switch (e->type) {
	case SDL_MOUSEBUTTONDOWN:
	    n = m->item->count;
	    for (i=0; i<n; i++) {
		widget_ptr w;
		it = m->item->item[i];
		w = (widget_ptr) it;

		if (widget_has_mouse(w)) if (widget_handle_event(w, e)) {
		    return 1;
		}
	    }
	    break;
    }
    return 0;
}

void menubar_update(widget_ptr w)
{
    menubar_ptr m = (menubar_ptr) w;
    menuitem_ptr it;
    int i, n;

    widget_fill(w, c_menubg);

    n = m->item->count;
    for (i=0; i<n; i++) {
	it = m->item->item[i];
	menuitem_update((widget_ptr) it);
    }
}

void menubar_init(menubar_ptr m)
{
    widget_init(m->widget);
    darray_init(m->item);
    m->widget->handle_event = menubar_handle_event;
    m->widget->update = menubar_update;
}

void menubar_clear(menubar_ptr m)
{
    darray_clear(m->item);
    widget_clear(m->widget);
}

void menubar_repack(menubar_t m)
{
    int i, n;
    int x;

    menuitem_ptr it;
    widget_ptr w = (widget_ptr) m;

    x = 0;
    n = m->item->count;

    for (i=0; i<n; i++) {
	it = m->item->item[i];
	widget_put_local((widget_ptr) it, x, w->y);
	x += ((widget_ptr) it)->w;
    }
}

void menubar_add(menubar_t m, menuitem_t it)
{
    darray_append(m->item, it);
    it->widget.parent = (widget_ptr) m;
    menubar_repack(m);
}

void menu_repack(menu_ptr m)
{
    int i, n;
    int y;
    int wmax = 0;
    widget_ptr w;

    //moved widths so that they are equal
    //and align in a vertical menu

    y = 0;

    n = m->item->count;
    for (i=0; i<n; i++) {
	w = (widget_ptr) m->item->item[i];
	if (wmax < w->w) wmax = w->w;
    }

    for (i=0; i<n; i++) {
	w = (widget_ptr) m->item->item[i];
	widget_put_size(w, wmax, w->h);
	widget_put_local(w, menu_border, y + menu_border);
	y += menubar_h;
    }

    widget_put_size((widget_ptr) m,
	    wmax + 2 * menu_border, y + 2 * menu_border);
}

void menu_add(menu_ptr m, menuitem_t it)
{
    darray_append(m->item, it);
    it->widget.parent = (widget_ptr) m;
    menu_repack(m);
}

void menu_moved(widget_ptr w)
{
    menu_ptr m = (menu_ptr) w;
    int i, n;

    widget_moved(w);
    n = m->item->count;
    for (i=0; i<n; i++) {
	menuitem_ptr it;
	widget_ptr wi;

	it = m->item->item[i];
	wi = (widget_ptr) it;
	widget_notify_move(wi);
    }
}

void menu_update(widget_ptr w)
{
    menu_ptr m = (menu_ptr) w;
    menuitem_ptr it;
    int i, n;
    SDL_Rect r;

    widget_fill(w, c_menuborder);
    r.x = menu_border;
    r.y = menu_border;
    r.w = w->w - menu_border * 2;
    r.h = w->h - menu_border * 2;
    widget_fillrect(w, &r, c_menubg);

    n = m->item->count;
    for (i=0; i<n; i++) {
	it = m->item->item[i];
	menuitem_update((widget_ptr) it);
    }
}

int menu_handle_event(widget_ptr p, event_ptr e)
{
    int i, n;
    menu_ptr m = (menu_ptr) p;
    menuitem_ptr it;

    switch (e->type) {
	case SDL_MOUSEBUTTONDOWN:
	    n = m->item->count;
	    for (i=0; i<n; i++) {
		widget_ptr w;
		it = m->item->item[i];
		w = (widget_ptr) it;

		if (widget_handle_event(w, e)) {
		    return 1;
		}
	    }
	    break;
    }
    return 0;
}

void menu_init(menu_ptr m)
{
    widget_ptr w = (widget_ptr) m;
    widget_init(w);
    w->can_focus = 1;
    w->moved = menu_moved;
    w->update = menu_update;
    w->handle_event = menu_handle_event;
    darray_init(m->item);
}

void menu_clear(menu_ptr m)
{
    widget_ptr w = (widget_ptr) m;
    darray_clear(m->item);
    widget_clear(w);
}

menu_ptr menu_new()
{
    menu_ptr r;

    r = (menu_ptr) malloc(sizeof(struct menu_s));
    menu_init(r);
    return r;
}

void menu_popup(menu_ptr m)
{
    widget_focus((widget_ptr) m);
}
