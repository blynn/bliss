#include <stdlib.h>
#include <math.h>
#include "machine_area.h"
#include "menu.h"
#include "mlist.h"
#include "util.h"

enum {
    disc_r = 10,
};

enum {
    drag_none = 0,
    drag_move,
    drag_edge
};

static int in_machine(int x, int y, machine_ptr m)
{
    return m->x <= x && x < m->x + m_w
	    && m->y <= y && y < m->y + m_h;
}

static int in_edge(int x, int y, edge_ptr e)
{
    int xd, yd;
    int d2;

    xd = 2 * x - (e->src->x + e->dst->x + m_w);
    yd = 2 * y - (e->src->y + e->dst->y + m_h);
    d2 = xd * xd + yd * yd;
    return d2 < disc_r * disc_r * 4;
}

static edge_ptr edge_at(machine_area_ptr ma, int x, int y)
{
    int i, n;
    edge_ptr e;
    darray_ptr a = ma->song->edge;

    n = a->count;
    for (i=0; i<n; i++) {
	e = (edge_ptr) a->item[i];
	if (in_edge(x, y, e)) {
	    return e;
	}
    }
    return NULL;
}

static machine_ptr machine_at(machine_area_ptr ma, int x, int y)
{
    int i, n;
    machine_ptr m;
    darray_ptr a = ma->zorder;

    n = a->count;
    for (i=0; i<n; i++) {
	m = (machine_ptr) a->item[i];
	if (in_machine(x, y, m)) {
	    //change z-orders
	    int j;
	    for (j=i; j<n-1; j++) {
		a->item[j] = a->item[j+1];
	    }
	    a->item[n-1] = m;
	    return m;
	}
    }
    return NULL;
}

static void try_connect(machine_area_ptr ma, machine_ptr dst)
{
    machine_ptr src = ma->drag_machine;
    if ((src->mi->type & machine_out) && (dst->mi->type & machine_in)) {
	if (song_is_connected(ma->song, src, dst)) return;
	song_create_edge(ma->song, src, dst);
    }
}

static void start_drag(widget_ptr w, int drag_type)
{
    machine_area_ptr ma = (machine_area_ptr) w;
    widget_getmousexy(w, &ma->drag_startx, &ma->drag_starty);
    ma->drag_machine = machine_at(ma, ma->drag_startx, ma->drag_starty);
    if (ma->drag_machine) {
	ma->drag_flag = drag_type;
    }
}

static void stop_drag(widget_ptr w)
{
    machine_area_ptr ma = (machine_area_ptr) w;
    if (ma->drag_flag == drag_edge) {
	int x, y;
	machine_ptr m;

	widget_getmousexy(w, &x, &y);
	m = machine_at(ma, x, y);
	if (m) {
	    try_connect(ma, m);
	}
    }
    ma->drag_flag = drag_none;
    ma->drag_machine = NULL;
}

static void new_machine(machine_area_ptr ma, char *id)
{
    machine_ptr m;
    m = song_create_machine_auto_id(ma->song, id);
    if (!m) return; //TODO: handle error
    darray_append(ma->zorder, m);
    widget_getmousexy((widget_ptr) ma, &m->x, &m->y);
}

static void popup_menu(widget_ptr w)
{
    machine_ptr m;
    edge_ptr e;
    machine_area_ptr ma = (machine_area_ptr) w;
    int x, y;

    stop_drag(w);
    widget_getmousexy(w, &x, &y);
    m = machine_at(ma, x, y);
    if (m) {
	widget_put_local((widget_ptr) ma->machmenu, x, y);
	ma->drag_machine = m;
	menu_popup(ma->machmenu);
	return;
    }
    e = edge_at(ma, x, y);
    if (e) {
	ma->sel_edge = e;
	widget_put_local((widget_ptr) ma->edgemenu, x, y);
	menu_popup(ma->edgemenu);
	return;
    }

    widget_put_local((widget_ptr) ma->rootmenu, x, y);
    menu_popup(ma->rootmenu);
}

int machine_area_handle_event(widget_ptr w, event_ptr e)
{
    machine_area_ptr ma = (machine_area_ptr) w;
    switch (e->type) {
	case SDL_MOUSEBUTTONDOWN:
	    if (e->button.button == SDL_BUTTON_RIGHT) {
		popup_menu(w);
	    } else if (e->button.button == SDL_BUTTON_LEFT) {
		if (widget_getmod(w) & KMOD_SHIFT) {
		    start_drag(w, drag_edge);
		} else {
		    start_drag(w, drag_move);
		}
	    }
	    break;
	case SDL_MOUSEBUTTONUP:
	    if (ma->drag_flag) stop_drag(w);
	    break;
    }
    return 1;
}

static void draw_edge(widget_ptr w, edge_ptr e)
{
    int x1, y1, x2, y2;
    static double cos30 = 0.0;
    double ds, dc;
    double a;
    int xc, yc;
    int xtri[3], ytri[3];

    x1 = e->src->x;
    y1 = e->src->y;
    x2 = e->dst->x;
    y2 = e->dst->y;

    x1 += m_w / 2;
    x2 += m_w / 2;
    y1 += m_h / 2;
    y2 += m_h / 2;

    if (!cos30) cos30 = cos(M_PI/6);

    //draw line between machines
    widget_line(w, x1, y1, x2, y2, c_edge);

    //draw triangle and circle
    a = atan(((double) (y1 - y2)) / ((double) (x2 - x1)));
    if (x2 < x1) a = M_PI + a;

    xc = (x1 + x2) / 2;
    yc = (y1 + y2) / 2;

    widget_filled_circle(w, xc, yc, disc_r, c_edgedisc);
    widget_circle(w, xc, yc, 9, c_edge);

    ds = sin(a) * (disc_r - 1);
    dc = cos(a) * (disc_r - 1);

    //(-ds,-dc) to go up one unit
    //(dc,-ds) to go right one unit
    xtri[0] = xc - floor(ds * cos30 + dc * 0.5);
    ytri[0] = yc - floor(dc * cos30 - ds * 0.5);

    xtri[1] = xc + dc;
    ytri[1] = yc - ds;

    xtri[2] = xtri[0] + ds * 2 * cos30;
    ytri[2] = ytri[0] + dc * 2 * cos30;

    widget_filled_polygon(w, xtri, ytri, 3, c_arrow);
}

static void draw_machine(widget_ptr w, machine_ptr m)
{
    machine_area_ptr ma = (machine_area_ptr) w;
    SDL_Rect r;
    int c;

    if (m == ma->drag_machine && ma->drag_flag == drag_move) {
	int x, y;
	widget_getmousexy(w, &x, &y);
	m->x += x - ma->drag_startx;
	m->y += y - ma->drag_starty;
	ma->drag_startx = x;
	ma->drag_starty = y;
    }

    r.x = m->x;
    r.y = m->y;
    r.w = m_w;
    r.h = m_h;
    widget_fillrect(w, &r, c_border);
    r.x++;
    r.y++;
    r.w -= 2;
    r.h -= 2;
    switch (m->mi->type) {
	case machine_master:
	    c = c_master;
	    break;
	case machine_generator:
	    c = c_generator;
	    break;
	case machine_effect:
	    c = c_effect;
	    break;
	default:
	    c = c_master;
	    break;
    }

    widget_fillrect(w, &r, c);
    widget_write(w, m->x + 4, m->y + 4, m->id);
}

void machine_area_update(widget_ptr w)
{
    darray_ptr a;
    int i, n;
    machine_ptr m;
    edge_ptr e;
    machine_area_ptr ma = (machine_area_ptr) w;

    a = ma->song->edge;
    n = a->count;
    for (i=0; i<n; i++) {
	e = a->item[i];
	draw_edge(w, e);
    }

    a = ma->zorder;
    n = a->count;
    for (i=0; i<n; i++) {
	m = a->item[i];
	draw_machine(w, m);
    }

    if (ma->drag_flag == drag_edge) {
	int x, y;
	m = ma->drag_machine;
	widget_getmousexy(w, &x, &y);
	widget_line(w, m->x + m_w / 2,
		m->y + m_h / 2, x, y, c_liveedge);
    }
}

static void del_edge_cb(widget_ptr w, void *data)
{
    machine_area_ptr ma = (machine_area_ptr) data;
    edge_ptr e = ma->sel_edge;
    song_del_edge(ma->song, e);
}

static void rename_cb2(widget_ptr w, void *data)
{
    machine_ptr m = (machine_ptr) data;
    char *s = ((tbwin_ptr) w)->tb->text;
    free(m->id);
    m->id = strclone(s);
}

static void rename_machine_cb(widget_ptr caller, void *data)
{
    machine_area_ptr ma = (machine_area_ptr) data;
    machine_ptr m = ma->drag_machine;
    widget_ptr wt;
    ma->drag_machine = NULL;
    textbox_put_text(ma->tbwin->tb, m->id);
    tbwin_open(ma->tbwin);
    wt = (widget_ptr) ma->tbwin;
    widget_put_local(wt, m->x - wt->x / 2, m->y);
    tbwin_put_title(ma->tbwin, "Rename");
    widget_connect(wt, signal_activate, rename_cb2, m);
}

static void del_machine_cb(widget_ptr w, void *data)
{
    machine_area_ptr ma = (machine_area_ptr) data;
    machine_ptr m = ma->drag_machine;
    ma->drag_machine = NULL;
    if (m == ma->song->master) {
	printf("can't delete master\n");
	return;
    }
    darray_remove(ma->zorder, m);
    song_del_machine(ma->song, m);
}

struct nm_s {
    machine_area_ptr ma;
    char *id;
};

struct nm_s *new_nm_s(machine_area_ptr ma, char *id)
{
    struct nm_s *r = (struct nm_s *) malloc(sizeof(struct nm_s));
    r->ma = ma;
    r->id = id;
    return r;
}

static void new_machine_cb(widget_ptr w, void *data)
{
    struct nm_s *r = (struct nm_s *) data;
    machine_area_ptr ma = r->ma;
    new_machine(ma, r->id);
}

static void new_pattern_cb(widget_ptr w, void *data)
{
    machine_area_ptr ma = (machine_area_ptr) data;
    machine_ptr m = ma->drag_machine;
    pattern_ptr p;

    p = machine_create_pattern_auto_id(m);
    root_edit_pattern(p);
    show_pattern_window_cb(w, NULL);
}

void machine_area_init(machine_area_ptr ma)
{
    widget_ptr w = (widget_ptr) ma;
    menuitem_ptr it;
    int i;

    widget_init(w);
    w->handle_event = machine_area_handle_event;
    w->update = machine_area_update;
    darray_init(ma->zorder);

    //popup menu for right click on machine
    menu_init(ma->machmenu);
    it = menuitem_new();
    menuitem_put_text(it, "Rename");
    menu_add(ma->machmenu, it);
    widget_connect((widget_ptr) it, signal_activate, rename_machine_cb, w);
    it = menuitem_new();
    menuitem_put_text(it, "Delete");
    menu_add(ma->machmenu, it);
    widget_connect((widget_ptr) it, signal_activate, del_machine_cb, w);
    it = menuitem_new();
    menuitem_put_text(it, "New Pattern");
    menu_add(ma->machmenu, it);
    widget_connect((widget_ptr) it, signal_activate, new_pattern_cb, w);

    //popup menu for right click on edge
    menu_init(ma->edgemenu);
    it = menuitem_new();
    menuitem_put_text(it, "Disconnect");
    menu_add(ma->edgemenu, it);
    widget_connect((widget_ptr) it, signal_activate, del_edge_cb, w);

    //popup menu for right click on background
    menu_init(ma->rootmenu);
    it = menuitem_new();
    menuitem_put_text(it, "New Machine");
    menu_add(ma->rootmenu, it);

    //list of all machines
    menu_init(ma->listmenu);
    menuitem_set_submenu(it, ma->listmenu);

    for (i=0; i<mlist->count; i++) {
	machine_info_ptr mi = (machine_info_ptr) mlist->item[i];
	if (mi->type == machine_master) continue;
	it = menuitem_new();
	menuitem_put_text(it, mi->id);
	menu_add(ma->listmenu, it);
	widget_connect((widget_ptr) it, signal_activate, new_machine_cb,
		new_nm_s(ma, mi->id));
    }

    //window for inputing single string
    tbwin_init(ma->tbwin);
}

void machine_area_clear(machine_area_ptr ma)
{
    widget_ptr w = (widget_ptr) ma;
    //TODO clear menu items
    //(use pool of menu items instead of dyn allocs)
    menu_clear(ma->edgemenu);
    menu_clear(ma->machmenu);
    menu_clear(ma->rootmenu);
    darray_clear(ma->zorder);
    widget_clear(w);
}

machine_area_ptr machine_area_new()
{
    machine_area_ptr r = (machine_area_ptr) malloc(sizeof(struct machine_area_s));
    machine_area_init(r);
    return r;
}

void machine_area_edit(machine_area_ptr ma, song_ptr song)
{
    ma->song = song;
    darray_copy(ma->zorder, song->machine);
}
