//graph editor
#include <SDL.h> //for SDL_ColorKey
#include <stdlib.h>
#include <math.h>

#include "gui.h"
#include "compan.h"
#include "orch.h"

widget_t canvas;

enum {
    //vd = "vertex distance" (in pixels)
    //  +------+
    //  |      |     +-+
    //  | unit +-----| | <-- box representing output port
    //  |      |     +-+ (input ports are similar but on the left side)
    //  +------+
    vd_h = 13,
    vd_w = 80,
    vd_textpad = 2,
    vd_edgew = 4,
    vd_edgey = 6, //distance from top to out edge
    vd_porty = 3, //distance from top to box representing port
    vd_portw = 7, //width of box representing port
    vd_porth = 7, //height
    vd_fudge = 2, //tolerance for error when user goes for a port with the mouse
};

struct saved_rect_s {
    widget_ptr w;
    rect_t r;
    image_ptr img;
};
typedef struct saved_rect_s saved_rect_t[1];
typedef struct saved_rect_s *saved_rect_ptr;

void saved_rect_get_ready(saved_rect_ptr p)
{
    p->img = NULL;
}

void restore_rect(saved_rect_ptr p)
{
    if (p->img) {
	screen_blit(p->img, NULL, p->r);
	append_request(p->r->x, p->r->y, p->r->w, p->r->h);
	image_free(p->img);
	p->img = NULL;
    }
}

void restore_and_save_rect(saved_rect_ptr p, widget_ptr w, rect_ptr r)
{
    if (p->img) restore_rect(p);
    p->w = w;
    p->r->x = w->globalx + r->x;
    p->r->y = w->globaly + r->y;
    p->r->w = r->w;
    p->r->h = r->h;
    p->img = image_new(r->w, r->h);
    screen_capture(p->r, p->img);
}

static int node_inport_count(node_ptr node)
{
    node_data_ptr p = node->data;
    int n = 0;

    //TODO: cumbersome. add inport count to node_data_ptr?
    if (p->type == node_type_normal || p->type == node_type_funk) {
	n = p->gen->info->port_count;
    }
    return n;
}

static int height_of_node(node_ptr node)
{
    return (node_inport_count(node) + 1) * vd_h;
}

//TODO: this is inefficient? midpoint already computed during update
static void compute_edge_midpoint(int *x, int *y, edge_ptr e)
{
    int x0, x1, y0, y1;
    node_ptr n0, n1;
    int portno = *((int *) e->data);

    n0 = e->src;
    n1 = e->dst;

    x0 = n0->x + vd_w + vd_edgew + vd_portw / 2;
    y0 = n0->y + vd_porty + vd_porth / 2;
    x1 = n1->x - vd_edgew - vd_portw / 2;
    y1 = n1->y + vd_porty + vd_porth / 2 + (portno + 1) * vd_h;

    *x = (x0 + x1) / 2;
    *y = (y0 + y1) / 2;
}

static int edge_contains(edge_ptr e, int x, int y)
{
    int dx, dy;

    compute_edge_midpoint(&dx, &dy, e);

    dx -= x;
    dy -= y;
    return dx * dx + dy * dy <= 12 * 12;
}

//TODO: ought to use a union?
static node_ptr node_selection = NULL;
static edge_ptr edge_selection = NULL;

static graph_ptr graph;

static int dragx, dragy;

#define SQRT3 1.732051

static image_ptr conimg;

static struct {
    void (*callback)(widget_ptr, void *, int x, int y);
    void *callback_data;
    image_ptr img, old;
    rect_t r;
    int flag;
} to_place;

static saved_rect_t srpotedge;

static int is_contained(int x, int y, int x0, int y0, int x1, int y1)
{
    if (x < x0) return 0;
    if (x > x1) return 0;
    if (y < y0) return 0;
    if (y > y1) return 0;
    return -1;
}

void canvas_put_graph(widget_ptr canvas, graph_ptr g)
{
    graph = g;
}

void canvas_select_nothing(widget_ptr canvas)
{
    edge_selection = NULL;
    node_selection = NULL;
    aux_show_nothing();
    //TODO: just need to move the selection cursor
    widget_update(canvas);
    request_update(canvas);
}

void canvas_select_edge(widget_ptr canvas, edge_ptr c)
{
    node_selection = NULL;
    edge_selection = c;
    aux_show_edge(c);
    //TODO: just need to move the selection cursor
    widget_update(canvas);
    request_update(canvas);
}

void canvas_select_node(widget_ptr canvas, node_ptr v)
{
    edge_selection = NULL;
    node_selection = v;
    //TODO: just need to move the selection cursor
    widget_update(canvas);
    request_update(canvas);
    aux_show_node(v);
}

static void draw_placement(widget_ptr canvas, int x0, int y0, int x1, int y1, void *data)
{
    if (to_place.flag) {
	screen_blit(to_place.old, NULL, to_place.r);
	to_place.flag = 0;
    }
    if (widget_contains(canvas, x1, y1)) {
	int x, y;

	x = x1 - to_place.r->w / 2;
	y = y1 - to_place.r->h / 2;
	to_place.r->x = x + canvas->globalx;
	to_place.r->y = y + canvas->globaly;
	if (to_place.r->x < 0) to_place.r->x = 0;
	/* TODO: fix this
	if (to_place.r->x + to_place.r->w >= root->w) {
	    to_place.r->x = root->w - to_place.r->w - 1;
	}
	*/
	screen_capture(to_place.r, to_place.old);
	widget_clip(canvas);
	widget_blit(canvas, x, y, to_place.img);
	//TODO: only need to update the img
	request_update(canvas);
	widget_unclip();
	to_place.flag = 1;
    } else {
	//TODO: only need to update the img
	request_update(canvas);
    }
}

static int try_connect(widget_ptr canvas, int button, int x, int y, void *data)
{
    int i, j, n;

    restore_rect(srpotedge);
    widget_unbind_mouse_motion(canvas);

    for (i=graph->node_list->count-1; i>=0; i--) {
	int x0, y0;
	node_ptr v = (node_ptr) graph->node_list->item[i];
	n = node_inport_count(v);

	y0 = v->y + vd_h + vd_porty;
	for (j=0; j<n; j++) {
	    x0 = v->x - vd_edgew - vd_portw;
	    if (is_contained(x, y, x0 - vd_fudge, y0 - vd_fudge,
			x0 + vd_portw - 1 + vd_fudge,
			y0 + vd_porth - 1 + vd_fudge)) {
		canvas_select_edge(canvas, add_edge(graph, node_selection, v, j));
		return 0;
	    }
	    y0 += vd_h;
	}
    }
    return 0;
}

static void drag_node(widget_ptr w, int xold, int yold, int x, int y, void *data)
{
    if (widget_contains(w, x, y)) {
	node_selection->x += x - dragx;
	node_selection->y += y - dragy;
	dragx = x;
	dragy = y;
    }
    widget_update(w);
    request_update(w);
}

static int finish_drag(widget_ptr canvas, int button, int x, int y, void *data)
{
    widget_unbind_mouse_motion(canvas);
    return 0;
}

static void draw_potentialedge(widget_ptr canvas,
	int xignore, int yignore, int x, int y,
	void *data)
{
    int x0, y0;
    int x1, y1;
    node_ptr v = node_selection;
    rect_t r;

    if (!widget_contains(canvas, x, y)) {
	restore_rect(srpotedge);
	return;
    }
    x0 = x - vd_portw / 2;
    y0 = y - vd_porth / 2;

    x1 = v->x + vd_w + vd_edgew + vd_portw / 2;
    y1 = v->y + vd_porty + vd_porth / 2;

    if (x0 < x1) r->x = x0; else r->x = x1;
    if (y0 < y1) r->y = y0; else r->y = y1;
    r->w = abs(x - x1) + vd_portw;
    r->h = abs(y - y1) + vd_porth;
    restore_and_save_rect(srpotedge, canvas, r);
    widget_box(canvas, x0, y0, x0 + vd_portw - 1, y0 + vd_porth - 1, c_darkedge);
    widget_line(canvas, x, y, x1, y1, c_darkedge);
    request_update_rect(canvas, r);
}

void canvas_placement_finish(widget_ptr canvas)
{
    to_place.callback = NULL;
    if (to_place.flag) {
	screen_blit(to_place.old, NULL, to_place.r);
	to_place.flag = 0;
    }
    image_free(to_place.img);
    image_free(to_place.old);
    widget_unbind_mouse_motion(canvas);

    widget_update(canvas);
    request_update(canvas);
}

static void canvas_handle_mousebuttondown(widget_ptr canvas, int button, int x, int y)
{
    int i;
    int vx = x - vd_w / 2;
    int vy = y - vd_h / 2;
    static int last_down_tick;

    if (to_place.callback) {
	to_place.callback(canvas, to_place.callback_data, vx, vy);
	compan_pop_all_but_one(compan);
	canvas_placement_finish(canvas);
	return;
    }

    for (i=graph->edge_list->count-1; i>=0; i--) {
	edge_ptr c = (edge_ptr) graph->edge_list->item[i];
	if (edge_contains(c, x, y)) {
	    canvas_select_edge(canvas, c);
	    return;
	}
    }

    for (i=graph->node_list->count-1; i>=0; i--) {
	int x0, y0;
	//check if node was clicked on
	node_ptr v = (node_ptr) graph->node_list->item[i];
	if (is_contained(x, y, v->x, v->y, v->x + vd_w - 1, v->y + height_of_node(v) - 1)) {
	    int now;
	    now = SDL_GetTicks();

	    if (v == node_selection && now - last_down_tick <= 250) {
		//double click on a node
		if (v == navoutnode) {
		    gui_back();
		    return;
		}
		node_data_ptr p = v->data;
		switch (p->type) {
		    case node_type_voice:
			gui_edit_voice(p->voice);
			break;
		    case node_type_ins:
			gui_edit_ins(p->ins);
			break;
		}
		return;
	    } else {
		//change z-order
		darray_remove_index(graph->node_list, i);
		darray_append(graph->node_list, v);

		canvas_select_node(canvas, v);
		dragx = x;
		dragy = y;
		widget_bind_mouse_motion(canvas, drag_node, NULL);
		widget_on_next_button_up(canvas, finish_drag, NULL);
	    }
	    last_down_tick = now;
	    return;
	}

	//check if output port was clicked on
	x0 = v->x + vd_w + vd_edgew;
	y0 = v->y + vd_porty;
	if (is_contained(x, y, x0, y0, x0 + vd_portw - 1, y0 + vd_porth)) {
	    widget_bind_mouse_motion(canvas, draw_potentialedge, NULL);
	    widget_on_next_button_up(canvas, try_connect, NULL);
	    saved_rect_get_ready(srpotedge);
	    canvas_select_node(canvas, v);
	    return;
	}
    }
    canvas_select_nothing(canvas);
    widget_update(canvas);
    request_update(canvas);
}

static void draw_selectioncursor(widget_ptr canvas)
{
    if (node_selection) {
	int x0, y0, x1, y1;
	x0 = node_selection->x - 1;
	y0 = node_selection->y - 1;
	x1 = x0 + vd_w + 1;
	y1 = y0 + height_of_node(node_selection) + 1;
	widget_rectangle(canvas, x0, y0, x1, y1, c_select);
    } else if (edge_selection) {
	int x, y;
	compute_edge_midpoint(&x, &y, edge_selection);
	widget_circle(canvas, x, y, 13, c_select);
    }
}

static void canvas_update(widget_ptr canvas)
{
    widget_clip(canvas);
    widget_box_rect(canvas, c_canvas);

    static void draw_edge(void *data)
    {
	edge_ptr e = data;
	int portno = *((int *) e->data);
	node_ptr n0, n1;
	int x0, y0, x1, y1;
	int x, y;
	int dx, dy;

	n0 = e->src;
	n1 = e->dst;

	//draw line connecting them
	x0 = n0->x + vd_w + vd_edgew + vd_portw / 2;
	y0 = n0->y + vd_porty + vd_porth / 2;
	x1 = n1->x - vd_edgew - vd_portw / 2;
	y1 = n1->y + vd_porty + vd_porth / 2 + (portno + 1) * vd_h;
	widget_line(canvas, x0, y0, x1, y1, c_edge);

	//compute midpoint for later
	//and dx, dy
	x = (x0 + x1) / 2;
	y = (y0 + y1) / 2;
	dx = x1 - x0;
	dy = y1 - y0;

	//draw closed port
	x0 = n1->x - vd_edgew - vd_portw;
	y0 = n1->y + vd_porty + (portno + 1) * vd_h;
	x1 = x0 + vd_portw - 1;
	y1 = y0 + vd_porth - 1;
	widget_box(canvas, x0, y0, x1, y1, c_edge);

	//draw thing in the middle
	{
	    int p0x, p0y;
	    int p1x, p1y;
	    int p2x, p2y;
	    double hyp;
	    double ds, dc;
	    widget_blit(canvas, x - 12, y - 12, conimg);
	    if (!dx && !dy) return;
	    hyp = sqrt(dx * dx + dy * dy);
	    ds = dy / hyp;
	    dc = dx / hyp;
	    p0x = x - 10 * (dc - ds * SQRT3) / 2;
	    p0y = y - 10 * (ds + SQRT3 * dc) / 2;
	    p1x = x + 10 * dc;
	    p1y = y + 10 * ds;
	    p2x = p0x - 10 * ds * SQRT3;
	    p2y = p0y + 10 * dc * SQRT3;

	    widget_filled_trigon(canvas, p0x, p0y, p1x, p1y, p2x, p2y, c_emphasis);
	}
    }

    static void draw_node(void *data)
    {
	int x0, x1, y0, y1;
	node_ptr v = data;
	node_data_ptr p = v->data;

	void draw_box(int c) {
	    widget_raised_border_box(canvas, x0, y0, x1, y1);
	    widget_box(canvas, x0 + 2, y0 + 2, x1 - 2, y1 - 2, c);
	}

	void draw_output_port() {
	    int y;
	    y = v->y + vd_edgey;
	    x0 = v->x + vd_w;
	    x1 = x0 + vd_edgew - 1;
	    widget_line(canvas, x0, y, x1, y, c_edge);
	    y = v->y + vd_porty;
	    x0 = x1 + 1;
	    x1 = x0 + vd_portw - 1;
	    widget_box(canvas, x0, y,
		    x1, y + vd_porth - 1, c_edge);
	}
	void draw_unit() {
	    int i, n;
	    gen_ptr g = p->gen;
	    n = g->info->port_count;

	    //draw input ports
	    y0 = v->y;
	    for (i=0; i<n; i++) {
		x0 = v->x;
		y0 += vd_h;
		//write name of input port
		widget_string(canvas, x0 + vd_textpad, y0 + vd_textpad, 
			g->info->port_name[i], c_porttext);

		//draw input "claw"
		widget_line(canvas, x0 - vd_edgew, y0 + vd_edgey,
			x0 - 1, y0 + vd_edgey, c_edge);
		x1 = x0 - vd_edgew - 1;
		x0 -= vd_edgew + vd_portw;
		y1 = y0 + vd_porty + vd_porth - 1;
		widget_line(canvas, x0, y0 + vd_porty,
			x1, y0 + vd_porty, c_edge);
		widget_line(canvas, x0, y1, x1, y1, c_edge);
		widget_line(canvas, x1, y0 + vd_porty, x1, y1, c_edge);
	    }
	}
	x0 = v->x;
	x1 = x0 + vd_w - 1;
	y0 = v->y;
	y1 = y0 + height_of_node(v) - 1;

	switch(p->type) {
	    case node_type_voice:
		draw_box(c_voice);
		break;
	    case node_type_ins:
		draw_box(c_ins);
		break;
	    default:
		if (v == navoutnode) {
		    draw_box(c_output);
		} else {
		    draw_box(c_unit);
		}
		draw_unit();
		break;
	}
	widget_string(canvas, v->x + 3, v->y + 2, p->id, c_emphasis);
	draw_output_port();
    }

    //draw vertices and edges
    darray_forall(graph->node_list, draw_node);
    darray_forall(graph->edge_list, draw_edge);

    draw_selectioncursor(canvas);
    widget_unclip(canvas);
}

static void canvas_placement_start(widget_ptr canvas,
	void (*callback)(widget_ptr, void *, int, int), void *data,
	int w, int h, char *s)
{
    widget_string(canvas, canvas->w / 2 - 40, canvas->h - 20,
	    s, c_emphasis);

    to_place.flag = 0;
    to_place.r->w = w;
    to_place.r->h = h;
    to_place.callback = callback;
    to_place.callback_data = data;
    to_place.img = image_new(to_place.r->w, to_place.r->h);
    to_place.old = image_new(to_place.r->w, to_place.r->h);
    image_box_rect(to_place.img, c_darkunit);
    widget_bind_mouse_motion(canvas, draw_placement, NULL);
}

//TODO: put this function somewhere
static node_ptr node_with_id(graph_ptr g, char *id)
{
    int i;
    for (i=0; i<g->node_list->count; i++) {
	node_ptr node = (node_ptr) g->node_list->item[i];
	node_data_ptr p = (node_data_ptr) node->data;
	if (!strcmp(p->id, id)) {
	    return node;
	}
    }
    return NULL;
}

static void place_ins(widget_ptr canvas, void *unused, int x, int y)
{
    int i;
    char id[80];
    node_data_ptr p;
    node_ptr v;

    for (i=0;;i++) {
	sprintf(id, "ins%d", i);
	if (!node_with_id(graph, id)) break;
    }

    v = orch_add_ins(orch, id, x, y);
    p = v->data;
    p->ins->out = add_ins_unit("out", out_uentry, p->ins,
	    canvas->w - 100, canvas->h / 2);
    canvas_select_node(canvas, v);
}

void canvas_place_ins_start(widget_ptr canvas)
{
    canvas_placement_start(canvas, place_ins, NULL,
	    vd_w, vd_h,
	    "Place Instrument");
}

static void place_voice(widget_ptr canvas, void *unused, int x, int y)
{
    int i;
    char id[80];
    node_ptr v;
    node_data_ptr p;

    for (i=0;;i++) {
	sprintf(id, "voice%d", i);
	if (!node_with_id(graph, id)) break;
    }

    v = add_voice(id, navthing, x, y);
    p = v->data;
    p->voice->out = add_voice_unit("out", out_uentry, p->voice,
	    canvas->w - 100, canvas->h / 2);
    add_voice_unit("freq", utable_at("dummy"), p->voice, 5, canvas->h / 2);
}

void canvas_place_voice_start(widget_ptr canvas)
{
    canvas_placement_start(canvas, place_voice, NULL,
	    vd_w, vd_h,
	    "Place Voice");
}

static void place_unit(widget_ptr canvas, void *data, int x, int y)
{
    int i;
    char id[80];
    uentry_ptr u = data;
    node_ptr v;

    for (i=0;;i++) {
	sprintf(id, "%s%d", u->namebase, i);
	if (!node_with_id(graph, id)) break;
    }
    if (navtype == nav_ins) {
	v = add_ins_unit(id, u, navthing, x, y);
    } else { //navtype == nav_voice
	v = add_voice_unit(id, u, navthing, x, y);
    }
}

void canvas_place_unit_start(widget_ptr canvas, uentry_ptr u)
{
    canvas_placement_start(canvas, place_unit, u,
	    vd_w,
	    (u->info->port_count + 1) * vd_h,
	    "Place Unit");
}

void canvas_init(widget_ptr canvas, widget_ptr parent)
{
    widget_init(canvas, parent);
    conimg = image_new(25, 25);
    SDL_SetColorKey(conimg, SDL_SRCCOLORKEY, 0);
    image_filled_circle(conimg, 12, 12, 12, c_shadow);
    image_filled_circle(conimg, 12, 12, 11, c_highlight);
    image_filled_circle(conimg, 12, 12, 10, c_unit);
    widget_show(canvas);
    canvas->update = canvas_update;
    canvas->handle_mousebuttondown = canvas_handle_mousebuttondown;
}

node_ptr canvas_current_node(widget_ptr canvas)
{
    return node_selection;
}

edge_ptr canvas_current_edge(widget_ptr canvas)
{
    return edge_selection;
}

void canvas_remove_current_node(widget_ptr canvas)
{
    node_data_ptr p = node_selection->data;
    p->clear(node_selection, p->clear_data);
    graph_remove_node(graph, node_selection);
    canvas_select_nothing(canvas);
    widget_update(canvas);
    request_update(canvas);
}

void canvas_remove_current_edge(widget_ptr canvas)
{
    graph_remove_edge(graph, edge_selection);
    canvas_select_nothing(canvas);
    widget_update(canvas);
    request_update(canvas);
}
