#include <SDL.h> //for SDL_ColorKey
#include <stdlib.h>
#include <math.h>

#include "canvas.h"

//many globals; can only have one canvas

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

//TODO: ought to use a union?
static vertex_ptr vertex_selection = NULL;
static connection_ptr connection_selection = NULL;

static void (*select_nothing_cb)();
static void (*select_connection_cb)(connection_ptr);
static void (*select_vertex_cb)(vertex_ptr);
static void (*connection_cb)(layout_ptr, vertex_ptr, vertex_ptr, int);

static layout_ptr layout;

static int dragx, dragy;

#define SQRT3 1.732051

static image_ptr conimg;

static struct {
    void (*callback)(void *, layout_ptr lp, int x, int y);
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

void canvas_put_layout(widget_ptr canvas, layout_ptr lp)
{
    layout = lp;
}

void canvas_select_nothing(widget_ptr canvas)
{
    connection_selection = NULL;
    vertex_selection = NULL;
    select_nothing_cb();
}

void canvas_select_connection(widget_ptr canvas, connection_ptr c)
{
    vertex_selection = NULL;
    connection_selection = c;
    select_connection_cb(c);
}

void canvas_select_vertex(widget_ptr canvas, vertex_ptr v)
{
    connection_selection = NULL;
    vertex_selection = v;
    select_vertex_cb(v);
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

    for (i=layout->vertex_list->count-1; i>=0; i--) {
	int x0, y0;
	vertex_ptr v = (vertex_ptr) layout->vertex_list->item[i];
	//gen_ptr g = ((node_data_ptr) v->node->data)->gen;
	//n = g->info->port_count;
	n = v->inportcount;
	y0 = v->w->localy + vd_h + vd_porty;
	for (j=0; j<n; j++) {
	    x0 = v->w->localx - vd_edgew - vd_portw;
	    if (is_contained(x, y, x0 - vd_fudge, y0 - vd_fudge,
			x0 + vd_portw - 1 + vd_fudge,
			y0 + vd_porth - 1 + vd_fudge)) {
		connection_cb(layout, vertex_selection, v, j);
		return 0;
	    }
	    y0 += vd_h;
	}
    }
    return 0;
}

static void drag_vertex(widget_ptr w, int xold, int yold, int x, int y, void *data)
{
    if (widget_contains(w, x, y)) {
	widget_translate(vertex_selection->w, x - dragx, y - dragy);
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
    vertex_ptr v = vertex_selection;
    rect_t r;

    if (!widget_contains(canvas, x, y)) {
	restore_rect(srpotedge);
	return;
    }
    x0 = x - vd_portw / 2;
    y0 = y - vd_porth / 2;

    x1 = v->w->localx + v->w->w + vd_edgew + vd_portw / 2;
    y1 = v->w->localy + vd_porty + vd_porth / 2;

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

    if (to_place.callback) {
	to_place.callback(to_place.callback_data, layout, vx, vy);
	canvas_placement_finish(canvas);
	return;
    }

    for (i=layout->connection_list->count-1; i>=0; i--) {
	connection_ptr c = (connection_ptr) layout->connection_list->item[i];
	if (local_contains(c->w, x, y)) {
	    canvas_select_connection(canvas, c);
	    return;
	}
    }

    for (i=layout->vertex_list->count-1; i>=0; i--) {
	int x0, y0;
	//check if vertex was clicked on
	vertex_ptr v = (vertex_ptr) layout->vertex_list->item[i];
	if (local_contains(v->w, x, y)) {
	    //change z-order
	    darray_remove_index(layout->vertex_list, i);
	    darray_append(layout->vertex_list, v);

	    canvas_select_vertex(canvas, v);
	    dragx = x;
	    dragy = y;
	    widget_bind_mouse_motion(canvas, drag_vertex, NULL);
	    widget_on_next_button_up(canvas, finish_drag, NULL);
	    return;
	}

	//check if output port was clicked on
	x0 = v->w->localx + v->w->w + vd_edgew;
	y0 = v->w->localy + vd_porty;
	if (is_contained(x, y, x0, y0, x0 + vd_portw - 1, y0 + vd_porth)) {
	    widget_bind_mouse_motion(canvas, draw_potentialedge, NULL);
	    widget_on_next_button_up(canvas, try_connect, NULL);
	    saved_rect_get_ready(srpotedge);
	    canvas_select_vertex(canvas, v);
	    return;
	}
    }
    canvas_select_nothing(canvas);
    widget_update(canvas);
    request_update(canvas);
}

static void draw_selectioncursor(widget_ptr canvas)
{
    widget_ptr w;
    if (vertex_selection) {
	w = vertex_selection->w;
	widget_rectangle(w, -1, -1, w->w, w->h, c_select);
    } else if (connection_selection) {
	w = connection_selection->w;
	widget_rectangle(w, -1, -1, w->w, w->h, c_select);
    }
}

static void canvas_update(widget_ptr canvas)
{
    widget_clip(canvas);
    widget_box_rect(canvas, c_canvas);

    static void draw_connection(void *data)
    {
	connection_ptr c = data;
	edge_ptr e = c->edge;
	int portno = *((int *) e->data);
	widget_ptr w0, w1;
	int x0, y0, x1, y1;
	int x, y;
	int dx, dy;

	w0 = c->src->w;
	w1 = c->dst->w;

	//draw line connecting them
	x0 = w0->localx + w0->w + vd_edgew + vd_portw / 2;
	y0 = w0->localy + vd_porty + vd_porth / 2;
	x1 = w1->localx - vd_edgew - vd_portw / 2;
	y1 = w1->localy + vd_porty + vd_porth / 2 + (portno + 1) * vd_h;
	widget_line(canvas, x0, y0, x1, y1, c_edge);

	//compute midpoint for later
	//and dx, dy
	x = (x0 + x1) / 2;
	y = (y0 + y1) / 2;
	dx = x1 - x0;
	dy = y1 - y0;
	c->w->localx = x - 12;
	c->w->localy = y - 12;
	c->w->globalx = c->w->localx + canvas->globalx;
	c->w->globaly = c->w->localy + canvas->globaly;
	c->w->w = 25;
	c->w->h = 25;

	//draw closed port
	x0 = - vd_edgew - vd_portw;
	y0 = vd_porty + (portno + 1) * vd_h;
	x1 = x0 + vd_portw - 1;
	y1 = y0 + vd_porth - 1;
	widget_box(w1, x0, y0, x1, y1, c_edge);

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

    static void draw_vertex(void *data)
    {
	vertex_ptr v = data;
	widget_update(v->w);
    }

    //draw vertices and connections
    darray_forall(layout->vertex_list, draw_vertex);
    darray_forall(layout->connection_list, draw_connection);

    draw_selectioncursor(canvas);
    widget_unclip(canvas);
}

void canvas_placement_start(widget_ptr canvas,
	void (*callback)(void *, layout_ptr, int, int), void *data,
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

void canvas_init(widget_ptr canvas, widget_ptr parent,
	void (*select_nothing_func)(),
	void (*select_connection_func)(connection_ptr),
	void (*select_vertex_func)(vertex_ptr),
	void (*connection_func)(layout_ptr, vertex_ptr, vertex_ptr, int))
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
    select_nothing_cb = select_nothing_func;
    select_connection_cb = select_connection_func;
    select_vertex_cb = select_vertex_func;
    connection_cb = connection_func;
}

vertex_ptr canvas_current_vertex(widget_ptr canvas)
{
    return vertex_selection;
}

connection_ptr canvas_current_connection(widget_ptr canvas)
{
    return connection_selection;
}

void canvas_remove_current_vertex(widget_ptr canvas)
{
    layout_remove_vertex(layout, vertex_selection);
    canvas_select_nothing(canvas);
    widget_update(canvas);
    request_update(canvas);
}

void canvas_remove_current_connection(widget_ptr canvas)
{
    layout_remove_connection(layout, connection_selection);
    canvas_select_nothing(canvas);
    widget_update(canvas);
    request_update(canvas);
}
