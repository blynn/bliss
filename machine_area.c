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
    for (i=n-1; i>=0; i--) {
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

/*
static void (*update_hook)(void *data);
static void* update_hook_data;

*/

static void context_command_list_init(context_command_ptr cc,
	int key, char *name, char *desc)
{
    cc->hotkey = key;
    cc->name = strclone(name);
    cc->desc = strclone(desc);
    cc->type = cc_list;
    darray_init(cc->list);
}

static void context_command_list_add(context_command_ptr cc, context_command_ptr c)
{
    //assert(cc->type == cc_list);
    darray_append(cc->list, c);
}

static void context_command_node_init(context_command_ptr cc,
	int key, char *name, char *desc, cc_callback_f f, void *data)
{
    cc->hotkey = key;
    cc->name = strclone(name);
    cc->desc = strclone(desc);
    cc->type = cc_node;
    cc->func = f;
    cc->data = data;
    darray_init(cc->list);
}

static void context_command_clear(context_command_ptr cc)
{
    switch(cc->type) {
	case cc_node:
	    free(cc->name);
	    free(cc->desc);
	    break;
	case cc_list:
	    free(cc->name);
	    free(cc->desc);
	    darray_clear(cc->list);
	    break;
	default:
	    fprintf(stderr, "unhandled CC type\n");
	    exit(1);
	    break;
    }
}

static void context_menu_clear(menu_ptr menu, darray_ptr ml, darray_ptr mil)
{
    int i;
    for (i=0; i<mil->count; i++) {
	menuitem_ptr it = (menuitem_ptr) mil->item[i];
	menuitem_clear(it);
	free(it);
    }

    for (i=0; i<ml->count; i++) {
	menu_ptr m = (menu_ptr) ml->item[i];
	menu_clear(m);
	free(m);
    }
    menu_remove_all(menu);
    darray_remove_all(ml);
    darray_remove_all(mil);
}

static void menu_cb(widget_ptr caller, void *data)
{
    context_command_ptr cc = (context_command_ptr) data;
    switch(cc->type) {
	case cc_node:
	    cc->func(cc->data);
	    break;
	case cc_list:
	    //submenu
	    break;
    }
}

static void context_menu_convert(menu_ptr menu, context_command_ptr cc,
	darray_ptr ml, darray_ptr mil)
{
    darray_ptr a;
    int i, n;
    menuitem_ptr it;
    menu_ptr m1;

    a = cc->list;
    n = a->count;

    for (i=0; i<n; i++) {
	context_command_ptr c = (context_command_ptr) cc->list->item[i];
	it = menuitem_new();
	darray_append(mil, it);
	menuitem_put_text(it, c->name);
	menu_add(menu, it);
	switch(c->type) {
	    case cc_node:
		widget_connect((widget_ptr) it, signal_activate, menu_cb, c);
		break;
	    case cc_list:
		m1 = menu_new();
		darray_append(ml, m1);
		context_menu_convert(m1, c, ml, mil);
		menuitem_set_submenu(it, m1);
		break;
	}
    }
}

static void set_current_menu(machine_area_ptr ma, context_command_ptr cc) {
    context_menu_clear(ma->menu, ma->menu_list, ma->menuitem_list);
    ma->cc_current = cc;
    context_menu_convert(ma->menu, cc, ma->menu_list, ma->menuitem_list);
}

static void clear_selection(machine_area_ptr ma)
{
    ma->sel_machine = NULL;
    ma->sel_edge = NULL;
    set_current_menu(ma, ma->cc_song);
}

static void update_selection(machine_area_ptr ma, int x, int y)
{
    ma->sel_machine = machine_at(ma, x, y);
    ma->sel_edge = edge_at(ma, x, y);

    // update context menus
    if (ma->sel_machine) {
	machine_ptr m = ma->sel_machine;
	if (m == ma->song->master) set_current_menu(ma, ma->cc_master);
	else set_current_menu(ma, ma->cc_machine);
    } else if (ma->sel_edge) {
	set_current_menu(ma, ma->cc_edge);
    } else {
	set_current_menu(ma, ma->cc_song);
    }
    /*
    if (update_hook) {
	update_hook(update_hook_data);
    }
    */
}

static void rename_sel_machine(void *data)
{
    machine_area_ptr ma = (machine_area_ptr) data;
    machine_ptr m = ma->sel_machine;
    widget_ptr wt;
    textbox_put_text(ma->tbwin->tb, m->id);
    tbwin_open(ma->tbwin);
    wt = (widget_ptr) ma->tbwin;
    widget_put_local(wt, m->x - wt->x / 2, m->y);
    tbwin_put_title(ma->tbwin, "Rename");
}

static void delete_sel_machine(void *data)
{
    machine_area_ptr ma = (machine_area_ptr) data;
    machine_ptr m = ma->sel_machine;
    clear_selection(ma);
    /*
    if (m == ma->song->master) {
	fprintf(stderr, "delete master request\n");
	exit(1);
    }
    */
    darray_remove(ma->zorder, m);
    song_del_machine(ma->song, m);
}

static void new_pattern_sel_machine(void *data)
{
    machine_area_ptr ma = (machine_area_ptr) data;
    machine_ptr m = ma->sel_machine;
    pattern_ptr p;

    p = machine_create_pattern_auto_id(m);
    root_edit_pattern(p);
    show_pattern_window_cb((widget_ptr) ma, NULL);
}

static void disconnect_sel_edge(void *data)
{
    machine_area_ptr ma = (machine_area_ptr) data;
    edge_ptr e = ma->sel_edge;
    song_del_edge(ma->song, e);
    clear_selection(ma);
}

static void machine_area_new_machine(machine_area_ptr ma, char *id)
{
    machine_ptr m;
    m = song_create_machine_auto_id(ma->song, id);
    if (!m) return; //TODO: handle error
    darray_append(ma->zorder, m);
    widget_getmousexy((widget_ptr) ma, &m->x, &m->y);
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

static void new_machine(void *data)
{
    struct nm_s *r = (struct nm_s *) data;
    machine_area_ptr ma = r->ma;
    machine_area_new_machine(ma, r->id);
}

static void init_menu(machine_area_ptr ma) {
    int i;

    menu_init(ma->menu);
    darray_init(ma->menu_list);
    darray_init(ma->menuitem_list);

    context_command_node_init(ma->cc_rename,
	    SDLK_r,
	    "Rename",
	    "Rename Selected Machine",
	    rename_sel_machine, ma);
    context_command_node_init(ma->cc_delete,
	    SDLK_DELETE,
	    "Delete",
	    "Delete Selected Machine",
	    delete_sel_machine, ma);
    context_command_node_init(ma->cc_new_pattern,
	    SDLK_p,
	    "New Pattern",
	    "Create New Pattern",
	    new_pattern_sel_machine, ma);
    context_command_node_init(ma->cc_disconnect,
	    SDLK_DELETE,
	    "Disconnect",
	    "Delete Connection",
	    disconnect_sel_edge, ma);
    context_command_list_init(ma->cc_new_machine,
	    SDLK_b,
	    "New Machine",
	    "New Machine");

    //list of all machines
    for (i=0; i<mlist->count; i++) {
	context_command_ptr cc;
	machine_info_ptr mi = (machine_info_ptr) mlist->item[i];
	if (mi->type == machine_master) continue;
	cc = (context_command_ptr) malloc(sizeof(context_command_t));

	context_command_node_init(cc,
		0,
		mi->id,
		mi->id,
		new_machine, new_nm_s(ma, mi->id));
	context_command_list_add(ma->cc_new_machine, cc);
    }

    context_command_list_init(ma->cc_song, 0, "Song", "Song Menu");
    context_command_list_add(ma->cc_song, ma->cc_new_machine);
    context_command_list_init(ma->cc_edge, 0, "Edge", "Edge Menu");
    context_command_list_add(ma->cc_edge, ma->cc_disconnect);
    context_command_list_init(ma->cc_machine, 0, "Machine", "Machine Menu");
    context_command_list_add(ma->cc_machine, ma->cc_rename);
    context_command_list_add(ma->cc_machine, ma->cc_delete);
    context_command_list_add(ma->cc_machine, ma->cc_new_pattern);
    context_command_list_init(ma->cc_master, 0, "Master", "Master Menu");
    context_command_list_add(ma->cc_master, ma->cc_rename);
    context_command_list_add(ma->cc_master, ma->cc_new_pattern);
}

/*
void machine_area_add_update_hook(void (*f)(void *), void *data)
{
    update_hook = f;
    update_hook_data = data;
}
*/

static void try_connect(machine_area_ptr ma, machine_ptr dst)
{
    machine_ptr src = ma->sel_machine;
    if (dst == src) return;
    if ((src->mi->type & machine_out) && (dst->mi->type & machine_in)) {
	if (song_is_connected(ma->song, src, dst)) return;
	song_create_edge(ma->song, src, dst);
    }
}

static void start_drag(widget_ptr w, int drag_type)
{
    machine_area_ptr ma = (machine_area_ptr) w;

    widget_getmousexy(w, &ma->drag_startx, &ma->drag_starty);
    update_selection(ma, ma->drag_startx, ma->drag_starty);
    if (ma->sel_machine) {
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
}

static void popup_menu(widget_ptr w)
{
    machine_area_ptr ma = (machine_area_ptr) w;
    int x, y;

    stop_drag(w);
    widget_getmousexy(w, &x, &y);
    update_selection(ma, x, y);
    widget_put_local((widget_ptr) ma->menu, w->x + x, w->y + y);
    menu_popup(ma->menu);
}

static int machine_area_handle_key(widget_ptr w, int key)
{
    machine_area_ptr ma = (machine_area_ptr) w;
    darray_ptr a = ma->cc_current->list;
    int i;

    for (i=0; i<a->count; i++) {
	context_command_ptr p = (context_command_ptr) a->item[i];
	if (key == p->hotkey) {
	    p->func(p->data);
	    return 1;
	}
    }
    return 0;
}

static int machine_area_handle_event(widget_ptr w, event_ptr e)
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
	case SDL_KEYDOWN:
	    return machine_area_handle_key(w, e->key.keysym.sym);
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
    machine_area_ptr ma = (machine_area_ptr) w;

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

    if (e == ma->sel_edge) {
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

static void draw_machine(widget_ptr w, machine_ptr m)
{
    machine_area_ptr ma = (machine_area_ptr) w;
    SDL_Rect r;
    int c;

    if (m == ma->sel_machine && ma->drag_flag == drag_move) {
	int x, y;
	widget_getmousexy(w, &x, &y);
	m->x += x - ma->drag_startx;
	m->y += y - ma->drag_starty;
	m->x = xclip(w, m->x);
	m->y = yclip(w, m->y);
	ma->drag_startx = x;
	ma->drag_starty = y;
    }

    if (m == ma->sel_machine) {
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
	    //ought to be a Bliss machine
	    c = c_bliss;
	    break;
    }

    widget_fillrect(w, &r, c);
    widget_write(w, m->x + 4, m->y + 4, m->id);
}

static void machine_area_update(widget_ptr w)
{
    darray_ptr a;
    int i, n;
    machine_ptr m;
    edge_ptr e;
    machine_area_ptr ma = (machine_area_ptr) w;

    //widget_fill(w, c_mabg);
    widget_draw_inverse_border(w);
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
	m = ma->sel_machine;
	widget_getmousexy(w, &x, &y);
	widget_line(w, m->x + m_w / 2,
		m->y + m_h / 2, x, y, c_liveedge);
    }
}

static void rename_cb2(widget_ptr w, void *data)
{
    machine_area_ptr ma = (machine_area_ptr) data;
    machine_ptr m = ma->sel_machine;
    char *s = ((tbwin_ptr) w)->tb->text;
    if (!song_machine_at(ma->song, s)) {
	free(m->id);
	m->id = strclone(s);
	root_status_text("Machine renamed");
    } else {
	root_status_text("Cannot rename: machine ID exists");
    }
}

void machine_area_init(machine_area_ptr ma)
{
    widget_ptr w = (widget_ptr) ma;

    widget_init(w);
    w->handle_event = machine_area_handle_event;
    w->update = machine_area_update;
    darray_init(ma->zorder);

    clear_selection(ma);

    init_menu(ma);

    //window for inputing single string
    tbwin_init(ma->tbwin);
    widget_connect((widget_ptr) ma->tbwin, signal_activate, rename_cb2, ma);
}

void machine_area_clear(machine_area_ptr ma)
{
    widget_ptr w = (widget_ptr) ma;
    context_menu_clear(ma->menu, ma->menu_list, ma->menuitem_list);
    //TODO: clear context_commands
    menu_clear(ma->menu);
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
    clear_selection(ma);
}

void machine_area_center(machine_area_ptr ma, machine_ptr m)
{
    widget_ptr w = (widget_ptr) ma;
    m->x = (w->w - m_w) / 2;
    m->y = (w->h - m_h) / 2;
}

void machine_area_put_buzz_coord(machine_area_ptr ma, machine_ptr m, double x, double y)
{
    widget_ptr w = (widget_ptr) ma;

    m->x = w->w * (x + 1) * 0.5;
    m->y = w->h * (y + 1) * 0.5;
}
