#include "menu.h"

enum {
    //md = "menu distance"
    //menu layout info in pixels
    md_padx = 8,
    md_pady = 7,
    md_h = 20,
    wd_border = 2,
};

static void menu_entry_update(widget_ptr w)
{
    int x, y;
    menu_entry_ptr e = (menu_entry_ptr) w;
    x = wd_border;
    y = wd_border;
    if (e->pushed) {
	widget_box_rect(w, c_menubg);
	widget_string(w, x + md_padx, y + md_pady,
		e->text, c_invtext);
    } else {
	widget_box_rect(w, c_background);
	widget_string(w, x + md_padx, y + md_pady,
		e->text, c_text);
    }
}

menu_entry_ptr menu_entry_new(menu_ptr m,
	char *s, void (*cb)(void *), void *data)
{
    menu_entry_ptr it;
    it = malloc(sizeof(menu_entry_t));
    widget_init(it->w, m->w);
    it->m = m;
    it->text = strclone(s);
    it->callback = cb;
    it->data = data;
    it->w->w = strlen(it->text) * 8 + 2 * md_padx;
    it->w->h = md_h;
    it->w->update = menu_entry_update;
    return it;
}

static void menu_update(widget_ptr w)
{
    widget_raised_background(w);
    widget_draw_children(w);
}

void menu_init(menu_ptr m, widget_ptr parent)
{
    widget_init(m->w, parent);
    m->w->update = menu_update;
    m->wmax = 0;
    m->w->w = 2 * wd_border;
    m->w->h = 2 * wd_border;
}

void menu_add_command(menu_ptr m, char *s, void (*func)(void *), void *data)
{
    menu_entry_ptr e = menu_entry_new(m, s, func, data);
    darray_append(m->w->child, e->w);
    e->w->localx = wd_border;
    e->w->localy = m->w->h - wd_border;
    m->w->h += e->w->h;
    if (m->wmax < e->w->w) {
	void set_wmax(void *data) {
	    menu_entry_ptr e = data;
	    e->w->w = m->wmax;
	}
	m->wmax = e->w->w;
	m->w->w = m->wmax + 2 * wd_border;
	darray_forall(m->w->child, set_wmax);
    } else {
	e->w->w = m->wmax;
    }
    widget_show(e->w);
}

void menubutton_update(widget_ptr w)
{
    menubutton_ptr m = (menubutton_ptr) w;
    if (m->pushed) {
	widget_box_rect(w, c_menubg);
	widget_string(w, md_padx, md_pady, m->text, c_invtext);
    } else {
	widget_box_rect(w, c_background);
	widget_string(w, md_padx, md_pady, m->text, c_text);
    }
}

static void menu_motion(widget_ptr w, int xold, int yold,
	int x, int y, void *data)
{
    menu_ptr menu = (menu_ptr) w;
    int i;
    if (menu->selected && local_contains(menu->selected, x, y)) {
	return;
    }
    for (i=0; i<w->child->count; i++) {
	widget_ptr wc =w->child->item[i];
	if (local_contains(wc, x, y)) {
	    if (menu->selected) {
		((menu_entry_ptr) menu->selected)->pushed = 0;
		widget_update(menu->selected);
		request_update(menu->selected);
	    }
	    menu->selected = wc;
	    ((menu_entry_ptr) wc)->pushed = -1;
	    widget_update(wc);
	    request_update(wc);
	    return;
	}
    }
}

static void menubutton_open(menubutton_ptr m)
{
    rect_t r;
    menu_ptr menu = m->menu;
    m->pushed = -1;
    menubutton_update(m->w);
    request_update(m->w);
    widget_show(menu->w);

    menu->under = image_new(menu->w->w, menu->w->h);
    r->x = menu->w->globalx;
    r->y = menu->w->globaly;
    r->w = menu->w->w;
    r->h = menu->w->h;
    image_blit_from_screen(menu->under, r);

    widget_update(menu->w);
    request_update(menu->w);

    menu->selected = NULL;
    widget_bind_mouse_motion(menu->w, menu_motion, NULL);
}

static void menubutton_close(menubutton_ptr m)
{
    menu_ptr menu = m->menu;
    widget_blit(menu->w, 0, 0, menu->under);
    image_free(menu->under);
    widget_hide(menu->w);
    request_update(menu->w);

    m->pushed = 0;
    menubutton_update(m->w);
    request_update(m->w);
    widget_unbind_mouse_motion(menu->w);

    if (menu->selected) {
	((menu_entry_ptr) menu->selected)->pushed = 0;
	menu->selected = NULL;
    }
}

void menubutton_init(menubutton_ptr m, widget_ptr parent, char *s)
{
    widget_init(m->w, parent);
    menu_init(m->menu, m->w);
    m->text = strclone(s);
    m->w->w = strlen(s) * 8 + 2 * md_padx;
    m->w->h = md_h - 2 * wd_border;
    m->w->update = menubutton_update;
    m->pushed = 0;
}

menubutton_ptr menubutton_new(widget_ptr parent, char *s)
{
    menubutton_ptr res = (menubutton_ptr) malloc(sizeof(menubutton_t));
    menubutton_init(res, parent, s);
    return res;
}

static void menubar_update(widget_ptr w)
{
    widget_raised_background(w);
    widget_draw_children(w);
}

static void menubar_motion(widget_ptr w, int xold, int yold,
	int x, int y, void *data)
{
    int i;
    menubar_ptr m = (menubar_ptr) w;

    if (local_contains(m->selected, x, y)) return;

    for (i=0; i<w->child->count; i++) {
	widget_ptr wc = w->child->item[i];
	if (local_contains(wc, x, y)) {
	    menubutton_close((menubutton_ptr) m->selected);
	    menubutton_open((menubutton_ptr) wc);
	    ((menubar_ptr) w)->selected = wc;
	    return;
	}
    }
}

static int menubar_mouse_button_up(widget_ptr w, int button, int x, int y, void *data)
{
    menubar_ptr m = (menubar_ptr) w;
    menubutton_ptr p = (menubutton_ptr) m->selected;
    menu_ptr menu = p->menu;
    widget_unbind_mouse_motion(w);
    if (menu->selected) {
	if (global_contains(menu->selected, x + w->globalx, y + w->globaly)) {
	    menu_entry_ptr e = (menu_entry_ptr) menu->selected;
	    menubutton_close(p);
	    e->callback(e->data);
	    return 0;
	}
    }
    menubutton_close(p);
    return 0;
}

static void menubar_mouse_button_down(widget_ptr w, int button, int x, int y)
{
    int i;
    for (i=0; i<w->child->count; i++) {
	widget_ptr wc = w->child->item[i];
	if (local_contains(wc, x, y)) {
	    menubutton_open((menubutton_ptr) wc);
	    ((menubar_ptr) w)->selected = wc;
	    widget_bind_mouse_motion(w, menubar_motion, NULL);
	    widget_on_next_button_up(w, menubar_mouse_button_up, NULL);
	    return;
	}
    }
}

void menubar_init(menubar_ptr m, widget_ptr parent)
{
    widget_init(m->w, parent);
    m->w->update = menubar_update;
    m->w->handle_mousebuttondown = menubar_mouse_button_down;
    m->w->h = md_h;
}

menu_ptr menubar_add_button(menubar_ptr m, char *s)
{
    menubutton_ptr res;
    widget_ptr w = m->w, wlast;
    int x;

    if (w->child->count) {
	wlast = darray_last(w->child);
	x = wlast->localx + wlast->w;
    } else {
	x = 0;
    }

    res = menubutton_new(m->w, s);
    res->w->localx = x;
    res->w->localy = wd_border;
    widget_show(res->w);
    res->menu->w->localx = m->w->localx;
    res->menu->w->localy = m->w->localy + m->w->h;
    return res->menu;
}
