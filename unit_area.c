#include <stdlib.h>
#include <math.h>
#include "unit_area.h"
#include "menu.h"
#include "mlist.h"
#include "util.h"
#include "bmachine.h"

enum {
    disc_r = 10,
};

enum {
    drag_none = 0,
    drag_move,
    drag_edge
};

enum {
    m_w = 64,
    m_h = 32
};

static int in_machine(int x, int y, unit_ptr m)
{
    return m->x <= x && x < m->x + m_w
	    && m->y <= y && y < m->y + m_h;
}

static int in_edge(int x, int y, unit_edge_ptr e)
{
    int xd, yd;
    int d2;

    xd = 2 * x - (e->src->x + e->dst->x + m_w);
    yd = 2 * y - (e->src->y + e->dst->y + m_h);
    d2 = xd * xd + yd * yd;
    return d2 < disc_r * disc_r * 4;
}

static unit_edge_ptr edge_at(unit_area_ptr ua, int x, int y)
{
    int i, n;
    unit_edge_ptr e;
    darray_ptr a = ua->mi->unit_edge;

    n = a->count;
    for (i=0; i<n; i++) {
	e = (unit_edge_ptr) a->item[i];
	if (in_edge(x, y, e)) {
	    return e;
	}
    }
    return NULL;
}

static unit_ptr unit_at(unit_area_ptr ua, int x, int y)
{
    int i, n;
    unit_ptr m;
    darray_ptr a = ua->zorder;

    n = a->count;
    for (i=n-1; i>=0; i--) {
	m = (unit_ptr) a->item[i];
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

/*
static void (*update_hook)(void *data);
static void* update_hook_data;

*/
static void clear_selection(unit_area_ptr ua)
{
    ua->sel_unit = NULL;
    ua->sel_edge = NULL;
}

static void update_selection(unit_area_ptr ua, int x, int y)
{
    ua->sel_unit = unit_at(ua, x, y);
    ua->sel_edge = edge_at(ua, x, y);
    /*
    if (update_hook) {
	update_hook(update_hook_data);
    }
    */
}

/*
void unit_area_add_update_hook(void (*f)(void *), void *data)
{
    update_hook = f;
    update_hook_data = data;
}
*/

static void try_connect(unit_area_ptr ua, unit_ptr dst)
{
    int sport, dport;
    unit_ptr src = ua->sel_unit;
    unit_edge_ptr e;

    if (dst == src) return;
    if ((dst->type == ut_param)) return;
    if ((src->type == ut_param)) {
	if (!(dst->type == ut_plugin)) return;
	//param --> plugin
printf("param --> plugin\n");
	sport = 0;
	dport = 2;
	//TODO: check if connected already
	e = (unit_edge_ptr) malloc(sizeof(struct unit_edge_s));
	e->src = src;
	e->dst = dst;
	e->srcport = sport;
	e->dstport = dport;
	darray_append(e->dst->in, e);
	darray_append(e->src->out, e);
	darray_append(ua->mi->unit_edge, e);
	return;
    }
    if ((src->type == ut_stream)) {
	if ((dst->type == ut_stream)) return;
	//stream --> plugin
printf("stream --> plugin\n");
	sport = 0;
	dport = 2;
	//TODO: check if connected already
	e = (unit_edge_ptr) malloc(sizeof(struct unit_edge_s));
	e->src = src;
	e->dst = dst;
	e->srcport = sport;
	e->dstport = dport;
	darray_append(e->dst->in, e);
	darray_append(e->src->out, e);
	darray_append(ua->mi->unit_edge, e);
	return;
    }
    if ((dst->type == ut_stream)) {
	//plugin --> stream
printf("plugin --> stream\n");
	sport = 0;
	dport = 0;
	//TODO: check if connected already
	e = (unit_edge_ptr) malloc(sizeof(struct unit_edge_s));
	e->src = src;
	e->dst = dst;
	e->srcport = sport;
	e->dstport = dport;
	darray_append(e->dst->in, e);
	darray_append(e->src->out, e);
	darray_append(ua->mi->unit_edge, e);
	return;
    }
    //plugin --> plugin
    for(sport=0 ;; sport++) {
	if (src->ui->port_desc[sport].type == up_output) break;
	if (sport >= src->ui->port_count) return;
    }
    for(dport=0 ;; dport++) {
	if (dst->ui->port_desc[dport].type == up_input) break;
	if (dport >= dst->ui->port_count) return;
    }
printf("plugin %d --> plugin %d\n", sport, dport);
    //TODO: check if connected already
    e = (unit_edge_ptr) malloc(sizeof(struct unit_edge_s));
    e->src = src;
    e->dst = dst;
    e->srcport = sport;
    e->dstport = dport;
    darray_append(e->dst->in, e);
    darray_append(e->src->out, e);
    darray_append(ua->mi->unit_edge, e);
    return;
}

static void start_drag(widget_ptr w, int drag_type)
{
    unit_area_ptr ua = (unit_area_ptr) w;

    widget_getmousexy(w, &ua->drag_startx, &ua->drag_starty);
    update_selection(ua, ua->drag_startx, ua->drag_starty);
    if (ua->sel_unit) {
	ua->drag_flag = drag_type;
    }
}

static void stop_drag(widget_ptr w)
{
    unit_area_ptr ua = (unit_area_ptr) w;
    if (ua->drag_flag == drag_edge) {
	int x, y;
	unit_ptr m;

	widget_getmousexy(w, &x, &y);
	m = unit_at(ua, x, y);
	if (m) {
	    try_connect(ua, m);
	}
    }
    ua->drag_flag = drag_none;
}

static void new_unit(unit_area_ptr ua, char *id)
{
    unit_ptr u;
    u = bmachine_create_unit_auto_id(ua->mi, id);
    if (!u) return; //TODO: handle error
    darray_append(ua->zorder, u);
    widget_getmousexy((widget_ptr) ua, &u->x, &u->y);
}

static void popup_menu(widget_ptr w)
{
    unit_ptr m;
    unit_edge_ptr e;
    unit_area_ptr ua = (unit_area_ptr) w;
    int x, y;

    stop_drag(w);
    widget_getmousexy(w, &x, &y);
    update_selection(ua, x, y);
    m = ua->sel_unit;
    if (m) {
	widget_put_local((widget_ptr) ua->machmenu, w->x + x, w->y + y);
	menu_popup(ua->machmenu);
	return;
    }
    e = ua->sel_edge;
    if (e) {
	widget_put_local((widget_ptr) ua->edgemenu, w->x + x, w->y + y);
	menu_popup(ua->edgemenu);
	return;
    }

    widget_put_local((widget_ptr) ua->rootmenu, w->x + x, w->y + y);
    menu_popup(ua->rootmenu);
}

static int unit_area_handle_event(widget_ptr w, event_ptr e)
{
    unit_area_ptr ua = (unit_area_ptr) w;
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
	    if (ua->drag_flag) stop_drag(w);
	    break;
	case SDL_KEYDOWN:
	    break;
    }
    return 1;
}

static void draw_edge(widget_ptr w, unit_edge_ptr e)
{
    int x1, y1, x2, y2;
    static double cos30 = 0.0;
    double ds, dc;
    double a;
    int xc, yc;
    int xtri[3], ytri[3];
    unit_area_ptr ua = (unit_area_ptr) w;

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

    if (e == ua->sel_edge) {
	widget_filled_circle(w, xc, yc, 12, c_machine_cursor);
    }
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

static int xclip(widget_ptr w, int x)
{
    if (x < 0) return 0;
    if (x + m_w > w->w) return w->w - m_w;
    return x;
}

static int yclip(widget_ptr w, int y)
{
    if (y < 0) return 0;
    if (y + m_h > w->h) return w->h - m_h;
    return y;
}

static void draw_machine(widget_ptr w, unit_ptr m)
{
    unit_area_ptr ua = (unit_area_ptr) w;
    SDL_Rect r;
    int c;

    if (m == ua->sel_unit && ua->drag_flag == drag_move) {
	int x, y;
	widget_getmousexy(w, &x, &y);
	m->x += x - ua->drag_startx;
	m->y += y - ua->drag_starty;
	m->x = xclip(w, m->x);
	m->y = yclip(w, m->y);
	ua->drag_startx = x;
	ua->drag_starty = y;
    }

    if (m == ua->sel_unit) {
	r.x = m->x - 2;
	r.y = m->y - 2;
	r.w = m_w + 4;
	r.h = m_h + 4;
	widget_fillrect(w, &r, c_machine_cursor);
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
    switch(m->type) {
	case ut_plugin:
	    c = c_generator;
	    break;
	case ut_param:
	    c = c_effect;
	    break;
	case ut_stream:
	    c = c_master;
	    break;
	default:
	    c = c_master;
	    break;
    }

    widget_fillrect(w, &r, c);
    widget_write(w, m->x + 4, m->y + 4, m->id);
}

static void unit_area_update(widget_ptr w)
{
    darray_ptr a;
    int i, n;
    unit_ptr m;
    unit_edge_ptr e;
    unit_area_ptr ua = (unit_area_ptr) w;

    widget_draw_inverse_border(w);
    a = ua->mi->unit_edge;
    n = a->count;
    for (i=0; i<n; i++) {
	e = a->item[i];
	draw_edge(w, e);
    }

    a = ua->zorder;
    n = a->count;
    for (i=0; i<n; i++) {
	m = a->item[i];
	draw_machine(w, m);
    }

    if (ua->drag_flag == drag_edge) {
	int x, y;
	m = ua->sel_unit;
	widget_getmousexy(w, &x, &y);
	widget_line(w, m->x + m_w / 2,
		m->y + m_h / 2, x, y, c_liveedge);
    }
}

static void del_edge_cb(widget_ptr w, void *data)
{
    unit_area_ptr ua = (unit_area_ptr) data;
    unit_edge_ptr e = ua->sel_edge;

    darray_remove(e->src->out, e);
    darray_remove(e->dst->in, e);
    darray_remove(ua->mi->unit_edge, e);
    free(e);

    clear_selection(ua);
}

static void rename_cb2(widget_ptr w, void *data)
{
    unit_area_ptr ua = (unit_area_ptr) data;
    unit_ptr m = ua->sel_unit;
    char *s = ((tbwin_ptr) w)->tb->text;
    free(m->id);
    m->id = strclone(s);
}

static void rename_machine_cb(widget_ptr caller, void *data)
{
    unit_area_ptr ua = (unit_area_ptr) data;
    unit_ptr m = ua->sel_unit;
    widget_ptr wt;
    textbox_put_text(ua->tbwin->tb, m->id);
    tbwin_open(ua->tbwin);
    wt = (widget_ptr) ua->tbwin;
    widget_put_local(wt, m->x - wt->x / 2, m->y);
    tbwin_put_title(ua->tbwin, "Rename");
}

static void del_machine_cb(widget_ptr w, void *data)
{
    unit_area_ptr ua = (unit_area_ptr) data;
    unit_ptr m = ua->sel_unit;
    clear_selection(ua);
    darray_remove(ua->zorder, m);
    bmachine_remove_unit(ua->mi, m);
    //TODO: remove edges too
    unit_clear(m);
    free(m);
}

struct nu_s {
    unit_area_ptr ua;
    char *id;
};

struct nu_s *new_nu_s(unit_area_ptr ua, char *id)
{
    struct nu_s *r = (struct nu_s *) malloc(sizeof(struct nu_s));
    r->ua = ua;
    r->id = id;
    return r;
}

static void new_unit_cb(widget_ptr w, void *data)
{
    struct nu_s *r = (struct nu_s *) data;
    unit_area_ptr ua = r->ua;
    new_unit(ua, r->id);
}

static void new_param_cb(widget_ptr w, void *data)
{
    unit_area_ptr ua = (unit_area_ptr) data;
    unit_ptr u = unit_new(NULL, "note");
    u->type = ut_param;
    bmachine_add_unit(ua->mi, u);
    darray_append(ua->zorder, u);
    widget_getmousexy((widget_ptr) ua, &u->x, &u->y);
}

static void new_stream_cb(widget_ptr w, void *data)
{
    unit_area_ptr ua = (unit_area_ptr) data;
    unit_ptr u = unit_new(NULL, "out0");
    u->type = ut_stream;
    bmachine_add_unit(ua->mi, u);
    darray_append(ua->zorder, u);
    widget_getmousexy((widget_ptr) ua, &u->x, &u->y);
}

void unit_area_init(unit_area_ptr ua)
{
    widget_ptr w = (widget_ptr) ua;
    menuitem_ptr it;
    int i;

    widget_init(w);
    w->handle_event = unit_area_handle_event;
    w->update = unit_area_update;
    darray_init(ua->zorder);
    clear_selection(ua);

    //popup menu for right click on machine
    menu_init(ua->machmenu);
    it = menuitem_new();
    menuitem_put_text(it, "Rename");
    menu_add(ua->machmenu, it);
    widget_connect((widget_ptr) it, signal_activate, rename_machine_cb, w);
    it = menuitem_new();
    menuitem_put_text(it, "Delete");
    menu_add(ua->machmenu, it);
    widget_connect((widget_ptr) it, signal_activate, del_machine_cb, w);

    //popup menu for right click on edge
    menu_init(ua->edgemenu);
    it = menuitem_new();
    menuitem_put_text(it, "Disconnect");
    menu_add(ua->edgemenu, it);
    widget_connect((widget_ptr) it, signal_activate, del_edge_cb, w);

    //popup menu for right click on background
    menu_init(ua->rootmenu);
    it = menuitem_new();
    menuitem_put_text(it, "New Unit");
    menu_add(ua->rootmenu, it);

    //list of all units
    menu_init(ua->listmenu);
    menuitem_set_submenu(it, ua->listmenu);

    for (i=0; i<ulist->count; i++) {
	unit_info_ptr mi = (unit_info_ptr) ulist->item[i];
	it = menuitem_new();
	menuitem_put_text(it, mi->id);
	menu_add(ua->listmenu, it);
	widget_connect((widget_ptr) it, signal_activate, new_unit_cb,
		new_nu_s(ua, mi->id));
    }

    it = menuitem_new();
    menuitem_put_text(it, "New Param");
    menu_add(ua->rootmenu, it);
    widget_connect((widget_ptr) it, signal_activate, new_param_cb, w);

    it = menuitem_new();
    menuitem_put_text(it, "New Stream");
    menu_add(ua->rootmenu, it);
    widget_connect((widget_ptr) it, signal_activate, new_stream_cb, w);

    //window for inputing single string
    tbwin_init(ua->tbwin);
    widget_connect((widget_ptr) ua->tbwin, signal_activate, rename_cb2, w);
}

void unit_area_clear(unit_area_ptr ua)
{
    widget_ptr w = (widget_ptr) ua;
    //TODO clear menu items
    //(use pool of menu items instead of dyn allocs)
    menu_clear(ua->edgemenu);
    menu_clear(ua->machmenu);
    menu_clear(ua->rootmenu);
    darray_clear(ua->zorder);
    widget_clear(w);
}

unit_area_ptr unit_area_new()
{
    unit_area_ptr r = (unit_area_ptr) malloc(sizeof(struct unit_area_s));
    unit_area_init(r);
    return r;
}

void unit_area_edit(unit_area_ptr ua, machine_info_ptr mi)
{
    ua->mi = mi;
    darray_copy(ua->zorder, mi->unit);
    clear_selection(ua);
}

void unit_area_center(unit_area_ptr ua, unit_ptr m)
{
    widget_ptr w = (widget_ptr) ua;
    m->x = (w->w - m_w) / 2;
    m->y = (w->h - m_h) / 2;
}
