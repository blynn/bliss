#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <SDL.h>

#include "audio.h"
#include "colour.h"
#include "ins.h"
#include "midi.h"
#include "textbox.h"
#include "button.h"
#include "checkbox.h"
#include "window.h"

#include "about.h"
#include "file_window.h"
#include "version.h"

#include "SDL_gfxPrimitives.h" //TODO: move routines that need this outta here

#define SQRT3 1.732051

//vertex = widgetized node
struct vertex_s {
    widget_t w;
    node_ptr node;
};

typedef struct vertex_s vertex_t[1];
typedef struct vertex_s* vertex_ptr;

//connection keeps tracks of edges
//holds vertices as well for convenience
struct connection_s {
    widget_t w;
    vertex_ptr src, dst;
    edge_ptr edge;
};

typedef struct connection_s connection_t[1];
typedef struct connection_s *connection_ptr;

image_ptr conimg;

int lastmousex, lastmousey;

//list of vertices sorted by z-order
static darray_t layout_list;

vertex_ptr vertex_with_id(char *id)
{
    int i;
    for (i=0; i<layout_list->count; i++) {
	vertex_ptr v = (vertex_ptr) layout_list->item[i];
	ins_node_ptr p = (ins_node_ptr) v->node->data;
	if (!strcmp(p->id, id)) {
	    return v;
	}
    }
    return NULL;
}

static darray_t connection_list;

struct uentry_s {
    char *namebase;
    gen_info_ptr info;
    image_ptr img;
};
typedef struct uentry_s uentry_t[1];
typedef struct uentry_s *uentry_ptr;

darray_t utable;

uentry_ptr utable_at(char *id)
{
    int i;
    for (i=0; i<utable->count; i++) {
	uentry_ptr p = (uentry_ptr) utable->item[i];
	if (!strcmp(p->info->id, id)) {
	    return p;
	}
    }
    return NULL;
}

uentry_ptr to_place = NULL;
image_ptr to_placeimg, to_placeold;
rect_t to_placer;
int to_placeflag = 0;

extern gen_info_t seg_info;
extern gen_info_t adsr_info;
extern gen_info_t osc_info;
extern gen_info_t out_info;
extern gen_info_t lpf_info;
extern gen_info_t dummy_info;
gen_info_ptr funk_info_table[8];
extern gen_info_ptr funk_info_n(int);

static void init_utable()
{
    darray_init(utable);
    int i;
    for (i=0; i<8; i++) {
	funk_info_table[i] = funk_info_n(i);
    }

    void add_entry(char *namebase, gen_info_ptr info, image_ptr img)
    {
	uentry_ptr p = (uentry_ptr) malloc(sizeof(uentry_t));
	p->namebase = namebase;
	p->info = info;
	p->img = img;
	darray_append(utable, p);
    }
    add_entry("osc", osc_info, SDL_LoadBMP("sine.bmp"));
    add_entry("adsr", adsr_info, SDL_LoadBMP("adsr.bmp"));
    add_entry("seg", seg_info, SDL_LoadBMP("seg.bmp"));
    add_entry("f", funk_info_table[2], SDL_LoadBMP("fx.bmp"));
    add_entry("out", out_info, NULL);
    add_entry("dummy", dummy_info, NULL);
    add_entry("lpf", lpf_info, SDL_LoadBMP("lpf.bmp"));
}

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

enum {
    //md = "menu distance"
    //menu layout info in pixels
    md_padx = 8,
    md_pady = 7,
    md_h = 20,
};

void widget_put_geometry(widget_ptr wid, int x, int y, int w, int h)
{
    wid->localx = x;
    wid->localy = y;
    wid->w = w;
    wid->h = h;
}

int interrupted = 0;

//TODO: ought to use a union?
vertex_ptr vertex_selection = NULL;
connection_ptr connection_selection = NULL;
int dragx, dragy;

ins_ptr ins;

static widget_t canvas;

void vertex_init(vertex_ptr v, node_ptr node, int x, int y)
{
    int n;
    widget_init(v->w, canvas);
    v->node = node;
    v->w->localx = x;
    v->w->localy = y;
    v->w->x = x + canvas->x;
    v->w->y = y + canvas->y;

    gen_ptr g = ((ins_node_ptr) v->node->data)->gen;
    n = g->info->port_count;

    v->w->w = vd_w + 4;
    v->w->h = vd_h * (n + 1) + 4;
}

vertex_ptr vertex_new(node_ptr node, int x, int y)
{
    vertex_ptr res;
    res = malloc(sizeof(vertex_t));
    vertex_init(res, node, x, y);
    return res;
}

static void add_vertex(node_ptr node, int x, int y)
{
    darray_append(layout_list, vertex_new(node, x, y));
}

static connection_ptr add_connection(vertex_ptr src, vertex_ptr dst, int port)
{
    connection_ptr c = (connection_ptr) malloc(sizeof(connection_t));

    c->src = src;
    c->dst = dst;
    c->edge = ins_connect(ins, src->node, dst->node, port);
    darray_append(connection_list, c);
    return c;
}

enum {
    minroot_w = 600,
    minroot_h = 400,
    defroot_w = 640,
    defroot_h = 480,
};

static SDL_Surface *screen;

int state;

static widget_t root;
static widget_t compan;
static widget_t aux_rect;
static widget_t info_rect;
static widget_t menu_rect;

window_t about_window;
file_window_t file_window;
window_ptr displaywindow = NULL;

void open_window(window_ptr w)
{
    displaywindow = w;
    w->w->update(w->w);
}

void close_window()
{
    displaywindow = NULL;
    root->update(root);
}

textbox_ptr textbox_selection;

button_ptr button_selection;

static textbox_t auxtb;
static textbox_ptr tbpool[10];

static button_t auxbutton;

darray_ptr command_list;

static void remove_connection(connection_ptr c)
{
    darray_remove(connection_list, c);
    ins_disconnect(ins, c->edge);
    free(c);
}

static void put_size(int w, int h)
{
    menu_rect->localx = 0;
    menu_rect->localy = 0;
    menu_rect->h = md_h;
    menu_rect->w = w;

    canvas->localx = 0;
    canvas->localy = menu_rect->h;
    canvas->w = w;
    canvas->h = h - 144 - menu_rect->h;

    compan->w = 5 * 40 - 4 + 16;
    compan->h = 144;
    compan->localx = w - compan->w;
    compan->localy = h - compan->h;

    aux_rect->w = (w - compan->w) / 2;
    aux_rect->h = compan->h;
    aux_rect->localx = compan->localx - aux_rect->w;
    aux_rect->localy = compan->localy;

    auxtb->w->w = aux_rect->w - 10;

    info_rect->w = w - compan->w - aux_rect->w;
    info_rect->h = compan->h;
    info_rect->localx = 0;
    info_rect->localy = compan->localy;

    widget_move_children(root);
}

static void init_libs(void)
{
    int status;
    int flag;

    status = SDL_Init(SDL_INIT_EVERYTHING);
    if (status) {
	fprintf(stderr, "init: SDL_Init failed: %s\n", SDL_GetError());
	exit(-1);
    }
    atexit(SDL_Quit);

    //font_init();

    audio_init();

    SDL_WM_SetCaption("Bliss", "Bliss");
    SDL_WM_SetIcon(SDL_LoadBMP("icon.bmp"), NULL);

    flag = SDL_RESIZABLE | SDL_HWSURFACE | SDL_DOUBLEBUF;// | SDL_FULLSCREEN;
    screen = SDL_SetVideoMode(defroot_w, defroot_h, 0, flag);
    widget_set_screen(screen);

    /*
    printf("Screen:");
    if (screen->flags & SDL_RESIZABLE) {
	printf(" resizable");
    }
    if (screen->flags & SDL_HWSURFACE) {
	printf(" hardware surface");
    }
    if (screen->flags & SDL_DOUBLEBUF) {
	printf(" double buffered");
    }
    printf("\n");
    */

    colour_init(screen->format);

    SDL_EnableKeyRepeat(150, 50);
    /*
    {
	static unsigned char fnt[2304];
	FILE *fp;
	fp = fopen("6x9.fnt", "rb");
	fread(fnt, 1, 2304, fp);
	fclose(fp);
	gfxPrimitivesSetFont(fnt, 6, 9);
    }
    */
    return;
}

static void main_resize(int x, int y)
{
    int flag;

    if (x < minroot_w) x = minroot_w;
    if (y < minroot_h) y = minroot_h;
    SDL_FreeSurface(screen);
    flag = SDL_RESIZABLE | SDL_HWSURFACE | SDL_DOUBLEBUF; //SDL_FULLSCREEN);
    screen = SDL_SetVideoMode(x, y, 0, flag);
    widget_set_screen(screen);
    root->w = x; root->h = y;
    put_size(x, y);
}

void bliss_quit()
{
    state = state_quit;
}

int is_contained(int x, int y, int x0, int y0, int x1, int y1)
{
    if (x < x0) return 0;
    if (x > x1) return 0;
    if (y < y0) return 0;
    if (y > y1) return 0;
    return -1;
}

static void draw_command_panel()
{
    int i;

    widget_raised_background(compan);
    for (i=0; i<command_list->count; i++) {
	button_ptr p = (button_ptr) command_list->item[i];
	widget_ptr w = p->w;
	button_update(p);
	if (widget_contains(w, lastmousex, lastmousey)) {
	    widget_rectangle(w, -1, -1,  w->w, w->h, c_select);
	    widget_string(compan, 10, compan->h - 12, p->text, c_text);
	}
    }
}

static darray_ptr mainlist, cancellist;

static void put_command_list(darray_ptr a)
{
    command_list = a;
    draw_command_panel();
}

static void prepare_to_place(uentry_ptr u)
{
    to_place = u;

    widget_string(canvas, canvas->w / 2 - 40, canvas->h - 20,
	    "Place Unit", c_emphasis);

    to_placer->w = vd_w;
    to_placer->h = (to_place->info->port_count + 1) * vd_h;

    to_placeimg = image_new(to_placer->w, to_placer->h);
    to_placeold = image_new(to_placer->w, to_placer->h);
    image_box_rect(to_placeimg, c_darkunit);
}

static void draw_placement()
{
    int x0, y0;

    if (to_placeflag) {
	SDL_BlitSurface(to_placeold, NULL, screen, to_placer);
	to_placeflag = 0;
    }
    x0 = lastmousex - to_placer->w / 2;
    y0 = lastmousey - to_placer->h / 2;
    if (widget_contains(canvas, lastmousex, lastmousey)) {
	to_placer->x = x0;
	to_placer->y = y0;
	if (to_placer->x < 0) to_placer->x = 0;
	if (to_placer->x + to_placer->w >= root->w) {
	    to_placer->x = root->w - to_placer->w - 1;
	}
	SDL_BlitSurface(screen, to_placer, to_placeold, NULL);
	widget_clip(canvas);
	widget_blit(root, x0, y0, to_placeimg);
	widget_unclip();
	to_placeflag = 1;
    }
}

static void draw_canvas();
static void done_to_place()
{
    to_place = NULL;
    if (to_placeflag) {
	SDL_BlitSurface(to_placeold, NULL, screen, to_placer);
	to_placeflag = 0;
    }
    image_clear(to_placeimg);
    image_clear(to_placeold);
    draw_canvas();
}

static void new_unit(void *data)
{
    prepare_to_place((uentry_ptr) data);
    put_command_list(cancellist);
}


static void cancelbutton(void *data)
{
    done_to_place();
    put_command_list(mainlist);
}

static void delconcb(void *data);

static button_ptr new_command_button(int row, int col)
{
    button_ptr b;

    b = button_new(compan);

    widget_ptr w = b->w;
    w->localx = col * (32 + 4 + 4) + 8;
    w->localy = row * (32 + 4 + 4) + 8;
    w->w = 32 + 4;
    w->h = 32 + 4;

    return b;
}

static void init_command()
{
    button_ptr b;

    void button_from_unit(button_ptr b, char *id)
    {
	uentry_ptr p = utable_at(id);
	b->img = p->img;
	b->callback = new_unit;
	b->data = (void *) p;
    }

    widget_init(compan, root);
    command_list = mainlist = darray_new();
    b = new_command_button(0, 0);
    b->text = "Oscillator";
    button_from_unit(b, "osc");
    darray_append(mainlist, b);

    b = new_command_button(0, 1);
    b->text = "Function";
    button_from_unit(b, "funk2");
    darray_append(mainlist, b);

    b = new_command_button(0, 2);
    b->text = "ADSR";
    button_from_unit(b, "adsr");
    darray_append(mainlist, b);

    b = new_command_button(0, 3);
    b->text = "Segment";
    button_from_unit(b, "seg");
    darray_append(mainlist, b);

    b = new_command_button(0, 4);
    b->text = "Low-Pass Filter";
    button_from_unit(b, "lpf");
    darray_append(mainlist, b);

    cancellist = darray_new();
    b = new_command_button(1, 4);
    b->text = "Cancel";
    b->img = SDL_LoadBMP("cancel.bmp");
    b->callback = cancelbutton;
    b->data = NULL;
    darray_append(cancellist, b);
}

static void draw_vertex(vertex_ptr v)
{
    int x0, x1, y0, y1;
    int i, n;
    gen_ptr g = ((ins_node_ptr) v->node->data)->gen;
    widget_ptr w0;
    w0 = v->w;

    //draw box containing unit
    widget_raised_border(w0);
    widget_box(w0, 2, 2, w0->w - 3, w0->h - 3, c_unit);

    //draw input ports
    n = g->info->port_count;
    y0 = 0;
    for (i=0; i<n; i++) {
	y0 += vd_h;
	//write name of input port
	widget_string(w0, vd_textpad, y0 + vd_textpad, 
		g->info->port_name[i], c_porttext);

	//draw input "claw"
	widget_line(w0, -vd_edgew, y0 + vd_edgey, -1, y0 + vd_edgey, c_edge);
	x0 = -vd_edgew - vd_portw;
	x1 = -vd_edgew - 1;
	y1 = y0 + vd_porty + vd_porth - 1;
	widget_line(w0, x0, y0 + vd_porty,
		x1, y0 + vd_porty, c_edge);
	widget_line(w0, x0, y1, x1, y1, c_edge);
	widget_line(w0, x1, y0 + vd_porty, x1, y1, c_edge);
    }

    //write name of unit
    widget_string(w0, 4, 4, ((ins_node_ptr) v->node->data)->id, c_emphasis);

    //draw output port
    y0 = vd_edgey;
    x0 = w0->w;
    x1 = x0 + vd_edgew - 1;
    widget_line(w0, x0, y0, x1, y0, c_edge);
    x0 = x1 + 1;
    x1 = x0 + vd_portw - 1;
    widget_box(w0, x0, vd_porty,
	    x1, vd_porty + vd_porth - 1, c_edge);
}

static void draw_selectioncursor()
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

static void draw_potentialedge()
{
    int x = lastmousex - canvas->x;
    int y = lastmousey - canvas->y;
    int x0, y0;
    vertex_ptr v = vertex_selection;
    x0 = x - vd_portw / 2;
    y0 = y - vd_porth / 2;
    widget_box(canvas, x0, y0, x0 + vd_portw - 1, y0 + vd_porth - 1, c_darkedge);
    x0 = v->w->localx + v->w->w + vd_edgew + vd_portw / 2;
    y0 = v->w->localy + vd_porty + vd_porth / 2;
    widget_line(canvas, x, y, x0, y0, c_darkedge);
}

static void draw_connection(connection_ptr c)
{
    edge_ptr e = c->edge;
    int portno = *((int *) e->data);
    widget_ptr w0, w1;
    int x0, y0, x1, y1;
    int x, y;
    int dx, dy;

    w0 = c->src->w;
    w1 = c->dst->w;

    //draw line connecting them
    x0 = w0->x + w0->w + vd_edgew + vd_portw / 2;
    y0 = w0->y + vd_porty + vd_porth / 2;
    x1 = w1->x - vd_edgew - vd_portw / 2;
    y1 = w1->y + vd_porty + vd_porth / 2 + (portno + 1) * vd_h;
    widget_line(root, x0, y0, x1, y1, c_edge);

    //compute midpoint for later
    //and dx, dy
    x = (x0 + x1) / 2;
    y = (y0 + y1) / 2;
    dx = x1 - x0;
    dy = y1 - y0;
    c->w->x = x - 12;
    c->w->y = y - 12;
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
	widget_blit(root, x - 12, y - 12, conimg);
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

	filledTrigonColor(screen, p0x, p0y, p1x, p1y, p2x, p2y, colourgfx[c_emphasis]);
    }
}

static void draw_canvas()
{
    int i;
    int x = lastmousex;
    int y = lastmousey;

    widget_clip(canvas);
    widget_box_rect(canvas, c_canvas);

    //draw vertices
    for (i=0; i<layout_list->count; i++) {
	vertex_ptr v = (vertex_ptr) layout_list->item[i];
	draw_vertex(v);
    }

    //draw edges
    for (i=0; i<connection_list->count; i++) {
	connection_ptr c = (connection_ptr) connection_list->item[i];
	draw_connection(c);
    }

    if (state == state_drag_vertex
	    && widget_contains(canvas, lastmousex, lastmousey)) {
	widget_translate(vertex_selection->w, x - dragx, y - dragy);
	dragx = x;
	dragy = y;
    }
    if (state == state_drag_port) {
	draw_potentialedge();
    }
    draw_selectioncursor();
    widget_unclip(canvas);
}

darray_t aux_tblist;

static checkbox_t keycheckbox;
static button_t recordb, playb;

enum {
    ev_noteon = 0,
    ev_noteoff
};

struct event_s {
    int delta;
    int type;
    int x1, x2;
};
typedef struct event_s event_t[1];
typedef struct event_s *event_ptr;

darray_t track;

static int is_recording, last_tick, is_playing, play_i;

static void draw_info()
{
    widget_raised_background(info_rect);
    checkbox_update(keycheckbox);
    button_update(recordb);
    button_update(playb);
    widget_string(info_rect, 24, 8, "Keyboard Plays", c_text);
    if (is_recording) {
	widget_filled_circle(info_rect, 75, 55, 8, c_highlight);
    } else {
	widget_filled_circle(info_rect, 75, 55, 8, c_darkshadow);
    }
}

struct intstring {
    int i;
    char *s;
};

static void param_set(void *data)
{
    double d;
    struct intstring *p = (struct intstring *) data;
    node_ptr node = vertex_selection->node;
    sscanf(p->s, "%lf", &d);
    set_param(node, p->i, d);
}

static void cancel_param_set(void *data)
{
}

static struct intstring param_set_data[10];

static void draw_vertex_selection()
{
    vertex_ptr v = vertex_selection;
    node_ptr node = v->node;
    ins_node_ptr p = (ins_node_ptr) node->data;

    widget_string(aux_rect, 5, 5, p->id, c_text);

    if (node_type(node) == ins_type_funk) {
	textbox_put_string(auxtb, get_funk_program(node));
	textbox_update(auxtb);
	darray_remove_all(aux_tblist);
	darray_append(aux_tblist, auxtb);
    } else {
	int i, n;
	char s[80];
	gen_ptr g = p->gen;
	n = g->info->param_count;
	darray_remove_all(aux_tblist);
	for (i=0; i<n; i++) {
	    widget_string(aux_rect, 5, i * 20 + 20,
		g->info->param[i]->id, c_text);
	    tbpool[i]->w->x = aux_rect->w / 2;
	    tbpool[i]->w->y = i * 20 + 20;
	    tbpool[i]->w->h = 16;
	    tbpool[i]->w->w = 80;
	    widget_shift_rect(tbpool[i]->w, aux_rect);
	    sprintf(s, "%.3f", g->param[i]);
	    textbox_put_string(tbpool[i], s);
	    textbox_update(tbpool[i]);
	    tbpool[i]->cancel_cb = cancel_param_set;
	    tbpool[i]->ok_cb = param_set;
	    param_set_data[i].i = i;
	    param_set_data[i].s = tbpool[i]->s;
	    tbpool[i]->ok_cb_data = (void *) &param_set_data[i];
	    darray_append(aux_tblist, tbpool[i]);
	}
    }
}

static void draw_connection_selection()
{
    connection_ptr c = connection_selection;
    ins_node_ptr p0, p1;
    char s[80];

    p0 = (ins_node_ptr) c->src->node->data;
    p1 = (ins_node_ptr) c->dst->node->data;
    snprintf(s, 80, "edge: %s --> %s", p0->id, p1->id);
    widget_string(aux_rect, 5, 5, s, c_text);
    button_update(auxbutton);
}

static void draw_aux()
{
    widget_raised_background(aux_rect);

    if (vertex_selection) {
	draw_vertex_selection();
    } else if (connection_selection) {
	draw_connection_selection();
    }
}

static void select_connection(connection_ptr c)
{
    vertex_selection = NULL;
    connection_selection = c;
    draw_aux();
}

static void select_vertex(vertex_ptr v)
{
    connection_selection = NULL;
    vertex_selection = v;
    draw_aux();
}

static void select_nothing()
{
    connection_selection = NULL;
    vertex_selection = NULL;
    draw_aux();
}

static void delconcb(void *data)
{
    remove_connection(connection_selection);
    select_nothing();
    draw_canvas();
}

struct menu_item_s {
    widget_t w;
    char *text;
    void (*callback)(void *);
    void *data;
};
typedef struct menu_item_s menu_item_t[1];
typedef struct menu_item_s *menu_item_ptr;

darray_t menubar;
darray_t filemenu;
darray_t helpmenu;

struct menu_selection_s {
    widget_t w;
    menu_item_ptr menubar_item;
    darray_ptr menu;
    image_ptr under;
    rect_t under_r;
};

struct menu_selection_s menu_selection[1];

void menu_item_put_text(menu_item_ptr it, char *s)
{
    it->text = s;
    it->w->w = strlen(s) * 8 + 2 * md_padx;
}

menu_item_ptr menu_item_new(char *s, void (*cb)(void *), void *data)
{
    menu_item_ptr it;
    it = malloc(sizeof(menu_item_t));
    menu_item_put_text(it, s);
    it->callback = cb;
    it->data = data;
    return it;
}

static void openmenu(void *data)
{
    int i;
    int wmax = 0;
    int w;
    darray_ptr m = (darray_ptr) data;
    menu_item_ptr it;

    if (menu_selection->under) {
	SDL_BlitSurface(menu_selection->under, NULL, screen, menu_selection->under_r);
	SDL_FreeSurface(menu_selection->under);
	menu_selection->under = NULL;
    }

    menu_selection->menu = m;

    //compute bounding box
    for (i=0; i<m->count; i++) {
	it = (menu_item_ptr) m->item[i];
	w = it->w->w;
	if (w > wmax) wmax = w;
    }
    menu_selection->w->w = wmax + 4; //4 for the border
    menu_selection->w->h = m->count * md_h + 4;
    it = menu_selection->menubar_item;
    menu_selection->w->x = it->w->x;
    menu_selection->w->y = it->w->y + it->w->h;

    menu_selection->under = SDL_CreateRGBSurface(SDL_HWSURFACE,
	    menu_selection->w->w, menu_selection->w->h, 32,
	    screen->format->Rmask,
	    screen->format->Gmask,
	    screen->format->Bmask,
	    screen->format->Amask);
    menu_selection->under_r->x = menu_selection->w->x;
    menu_selection->under_r->y = menu_selection->w->y;
    menu_selection->under_r->w = menu_selection->w->w;
    menu_selection->under_r->h = menu_selection->w->h;
    SDL_BlitSurface(screen, menu_selection->under_r, menu_selection->under, NULL);
}

static void track_clear();
static void ins_editor_clear()
{
    int i;

    for (i=0; i<layout_list->count; i++) {
	vertex_ptr v = (vertex_ptr) layout_list->item[i];
	free(v);
    }
    for (i=0; i<connection_list->count; i++) {
	connection_ptr c = (connection_ptr) connection_list->item[i];
	free(c);
    }
    darray_clear(layout_list);
    darray_clear(connection_list);
    ins_clear(ins);
    track_clear();

    vertex_selection = NULL;
    connection_selection = NULL;
}

static void ins_editor_new()
{
    darray_init(layout_list);
    darray_init(connection_list);
    ins = ins_new("Ins1");
    add_vertex(ins->out, canvas->w - 100, canvas->h / 2);
    add_vertex(ins->freq, 5, canvas->h / 2);
}

static void savecb(void *data)
{
    char *s = (char *) data;
    close_window();
    {
	int i, j;
	FILE *fp;
	fp = fopen(s, "wb");
	fprintf(fp, "bliss %s\n", VERSION_STRING);
	for (i=0; i<layout_list->count; i++) {
	    vertex_ptr v = (vertex_ptr) layout_list->item[i];
	    //node_ptr node = (node_ptr) ins->node_list->item[i];
	    node_ptr node = v->node;
	    ins_node_ptr p = (ins_node_ptr) node->data;
	    gen_ptr g = p->gen;

	    fprintf(fp, "unit %s %s ", p->id, g->info->id);
	    fprintf(fp, "%d %d\n", v->w->localx, v->w->localy);
	    if (p->type == ins_type_funk) {
		fprintf(fp, "setfn %s %s\n", p->id, get_funk_program(node));
	    } else if (p->type == ins_type_normal) {
		for (j=0; j<g->info->param_count; j++) {
		    fprintf(fp, "set %s %s %f\n", p->id,
			    g->info->param[j]->id, g->param[j]);
		}
	    }
	}
	for (i=0; i<connection_list->count; i++) {
	    connection_ptr c = (connection_ptr) connection_list->item[i];
	    edge_ptr e = c->edge;
	    ins_node_ptr p0, p1;
	    p0 = (ins_node_ptr) e->src->data;
	    p1 = (ins_node_ptr) e->dst->data;
	    fprintf(fp, "connect %s %s %d\n", p0->id, p1->id, *((int *) e->data));
	}
	fprintf(fp, "track {\n");
	for (i=0; i<track->count; i++) {
	    event_ptr e = track->item[i];
	    fprintf(fp, "  %d ", e->delta);
	    switch(e->type) {
		case ev_noteon:
		    fprintf(fp, "noteon %d %d", e->x1, e->x2);
		    break;
		case ev_noteoff:
		    fprintf(fp, "noteoff %d", e->x1);
		    break;
	    }
	    fprintf(fp, "\n");
	}
	fprintf(fp, "}\n");
	fclose(fp);
    }
}

static void loadcb(void *data)
{
    //TODO: error handling
    char *filename = (char *) data;
    close_window();
    {
	int i;
	FILE *fp;
	char s[256];
	char s1[256];

	void read_word() {
	    int c;
	    i = 0;
	    for(;;) {
		c = fgetc(fp);
		if (c == EOF) {
		    s[i] = 0;
		    return;
		}
		if (!strchr(" \t\r\n", c)) break;
	    }
	    s[i++] = c;
	    for(;;) {
		c = fgetc(fp);
		if (c == EOF) {
		    s[i] = 0;
		    return;
		}
		if (strchr(" \t\r\n", c)){
		    s[i] = 0;
		    return;
		}
		s[i++] = c;
	    }
	}

	fp = fopen(filename, "rb");
	read_word();
	//TODO: check it's "bliss"
	read_word();
	//TODO: check version

	ins_editor_clear();
	ins = ins_new("Ins1");
	darray_init(layout_list);
	darray_init(connection_list);
	for (;;) {
	    read_word();
	    if (!strcmp(s, "unit")) {
		uentry_ptr u;
		node_ptr node;
		int x, y;

		read_word();
		strcpy(s1, s);
		read_word();
		u = utable_at(s);
		if (!u) {
		    printf("Unknown unit type!\n");
		    break;
		}
		node = ins_add_gen(ins, u->info, s1);
		read_word();
		x = atoi(s);
		read_word();
		y = atoi(s);
		add_vertex(node, x, y);
		if (!strcmp(s1, "out")) {
		    ins->out = node;
		} else if (!strcmp(s1, "freq")) {
		    ins->freq = node;
		}
	    } else if (!strcmp(s, "set")) {
		vertex_ptr v;
		int param;
		read_word();
		v = vertex_with_id(s);
		read_word();
		param = no_of_param(v->node, s);
		read_word();
		set_param(v->node, param, atof(s));
	    } else if (!strcmp(s, "setfn")) {
		vertex_ptr v;
		read_word();
		v = vertex_with_id(s);
		read_word();
		set_funk_program(v->node, s);
	    } else if (!strcmp(s, "connect")) {
		vertex_ptr v0, v1;
		int port;
		read_word();
		v0 = vertex_with_id(s);
		read_word();
		v1 = vertex_with_id(s);
		read_word();
		port = atoi(s);
		add_connection(v0, v1, port);
	    } else if (!strcmp(s, "track")) {
		read_word(); //TODO: check its '{'
		for(;;) {
		    event_ptr e;
		    read_word();
		    if (!strcmp(s, "}")) break;
		    e = malloc(sizeof(event_t));
		    e->delta = atoi(s);
		    read_word();
		    if (!strcmp(s, "noteon")) {
			e->type = ev_noteon;
			read_word();
			e->x1 = atoi(s);
			read_word();
			e->x2 = atoi(s);
		    } else if (!strcmp(s, "noteoff")) {
			e->type = ev_noteoff;
			read_word();
			e->x1 = atoi(s);
			e->x2 = 0;
		    }
		    darray_append(track, e);
		}
	    } else {
		break;
	    }
	}
	fclose(fp);
	draw_canvas();
	draw_aux();
    }
}

static void savemenuitemcb(void *data)
{
    file_window_setup(file_window, "Save File", "Save", savecb);
    open_window(file_window->win);
}

static void newmenuitemcb(void *data)
{
    ins_editor_clear();
    ins_editor_new();
    draw_canvas();
    draw_aux();
}

static void do_event(event_ptr);
static void rendermenuitemcb(void *data)
{
    FILE *fp;

    void write2(int n) {
	int m = n;
	fputc(m & 255, fp);
	m >>= 8;
	fputc(m & 255, fp);
    }

    void write4(int n) {
	int m = n;
	fputc(m & 255, fp);
	m >>= 8;
	fputc(m & 255, fp);
	m >>= 8;
	fputc(m & 255, fp);
	m >>= 8;
	fputc(m & 255, fp);
    }

    if (!track->count) return;

    fp = fopen("bliss.wav", "wb");
    fprintf(fp, "RIFF");
    fprintf(fp, "????");
    fprintf(fp, "WAVE");
    fprintf(fp, "fmt ");
    write4(16);
    write2(1);
    write2(2);
    write4(44100);
    write4(4 * 44100);
    write2(4);
    write2(16);
    fprintf(fp, "data");
    fprintf(fp, "????");

    SDL_PauseAudio(1);
    {

    int t = 0, t1;
    int i = 0;
    int n;
    event_ptr e = track->item[i];
    t1 = 0;
    for(;;) {
	while (t * 1000 / 44100 >= e->delta + t1) {
	    do_event(e);
	    t1 += e->delta;
	    i++;
	    if (i >= track->count) {
		goto nomore;
	    }
	    e = track->item[i];
	}
	n = ins_tick(ins) * 4096;
	write2(n);
	write2(n);
	t++;
    }
nomore:
    while (ins->note_list->count) {
	n = ins_tick(ins) * 4096;
	write2(n);
	write2(n);
	t++;
    }

    fseek(fp, 4L, SEEK_SET);
    write4(t * 4 + 36);
    fseek(fp, 40L, SEEK_SET);
    write4(t * 4);
    }
    SDL_PauseAudio(0);
    fclose(fp);
}

static void loadmenuitemcb(void *data)
{
    file_window_setup(file_window, "Open File", "Open", loadcb);
    open_window(file_window->win);
}

static void quitcb(void *data)
{
    bliss_quit();
}

static void aboutcb(void *data)
{
    open_window(about_window);
}

static void init_menu()
{
    menu_item_ptr it;

    darray_init(menubar);

    darray_init(filemenu);

    it = menu_item_new("Open...", loadmenuitemcb, NULL);
    darray_append(filemenu, it);

    it = menu_item_new("Save...", savemenuitemcb, NULL);
    darray_append(filemenu, it);

    it = menu_item_new("New", newmenuitemcb, NULL);
    darray_append(filemenu, it);

    it = menu_item_new("Render", rendermenuitemcb, NULL);
    darray_append(filemenu, it);

    it = menu_item_new("Quit", quitcb, NULL);
    darray_append(filemenu, it);

    darray_init(helpmenu);

    it = menu_item_new("About...", aboutcb, NULL);
    darray_append(helpmenu, it);

    it = menu_item_new("File", openmenu, filemenu);
    darray_append(menubar, it);

    it = menu_item_new("Help", openmenu, helpmenu);
    darray_append(menubar, it);

    {
	int i;
	int x = 2, y = 2; //2 for the border
	for (i=0; i<menubar->count; i++) {
	    menu_item_ptr it = (menu_item_ptr) menubar->item[i];
	    it->w->localx = x;
	    it->w->localy = y;
	    it->w->x = x;
	    it->w->y = y;
	    it->w->h = md_h - 2;
	    x += it->w->w;
	}
    }

    menu_selection->under = NULL;
}

static void draw_menu()
{
    int i;
    int x = 2; //2 for the border TODO: cleanup
    int y = md_pady;
    widget_raised_background(menu_rect);
    for (i=0; i<menubar->count; i++) {
	menu_item_ptr it = (menu_item_ptr) menubar->item[i];
	//check if mouse had moved over different menubar item
	if (widget_contains(it->w, lastmousex, lastmousey)
		&& it != menu_selection->menubar_item) {
	    menu_selection->menubar_item = it;
	    if (it->callback == openmenu) it->callback(it->data);
	}
	//draw menuitem
	if (state == state_menu && it == menu_selection->menubar_item) {
	    widget_box_rect(it->w, c_menubg);
	    widget_string(it->w, md_padx, md_pady, it->text, c_invtext);
	} else {
	    widget_string(it->w, md_padx, md_pady, it->text, c_text);
	}
    }

    if (state == state_menu && menu_selection->menu) {
	darray_ptr m = menu_selection->menu;
	int my = lastmousey - menu_selection->w->y;
	int flag = 0;

	x = 2; //2 for the border
	y = 2;
	widget_raised_background(menu_selection->w);
	if (widget_contains(menu_selection->w, lastmousex, lastmousey)) {
	    flag = 1;
	}
	for (i=0; i<m->count; i++) {
	    menu_item_ptr it = (menu_item_ptr) m->item[i];
	    if (flag && my >= y && my < y + md_h) {
		widget_box(menu_selection->w, 2, y,
			menu_selection->w->w - 4, y + md_h - 1, c_menubg);
		widget_string(menu_selection->w, x + md_padx, y + md_pady,
			it->text, c_invtext);
	    } else {
		widget_string(menu_selection->w, x + md_padx, y + md_pady,
			it->text, c_text);
	    }
	    y += md_h;
	}
    }
}

static int keyboardplayflag = 0;

static void root_handle_mousebuttondown(widget_ptr ignore, int button, int x, int y)
{
    if (state == state_textbox) {
	state = state_normal;
	textbox_update(textbox_selection);
	textbox_selection = NULL;
    }
    /*
    if (state == state_textbox) {
	if (widget_contains(textbox_selection->w, x, y)) {
	    textbox_handlembdown(textbox_selection, button,
		    x - textbox_selection->w->x, y - textbox_selection->w->y);
	    return;
	} 
	textbox_ok(textbox_selection);
    }
    */

    if (widget_contains(menu_rect, x, y)) {
	int i;
	for (i=0; i<menubar->count; i++) {
	    menu_item_ptr it = (menu_item_ptr) menubar->item[i];
	    if (widget_contains(it->w, x, y)) {
		state = state_menu;
		menu_selection->menubar_item = it;
		menu_selection->menu = NULL;
		if (it->callback == openmenu) it->callback(it->data);
		return;
	    }
	}
    } else if (widget_contains(canvas, x, y)) {
	int i;

	if (to_place) {
	    char id[80];
	    int vx = x - canvas->x - vd_w / 2;
	    int vy = y - canvas->y - vd_h / 2;
	    for (i=0;;i++) {
		sprintf(id, "%s%d", to_place->namebase, i);
		if (!vertex_with_id(id)) break;
	    }
	    add_vertex(ins_add_gen(ins, to_place->info, id), vx, vy);
	    select_vertex(darray_last(layout_list));
	    done_to_place();
	    put_command_list(mainlist);
	    return;
	}

	for (i=connection_list->count-1; i>=0; i--) {
	    connection_ptr c = (connection_ptr) connection_list->item[i];
	    if (widget_contains(c->w, x, y)) {
		select_connection(c);
		draw_canvas();
		return;
	    }
	}

	for (i=layout_list->count-1; i>=0; i--) {
	    int x0, y0;
	    //check if vertex was clicked on
	    vertex_ptr v = (vertex_ptr) layout_list->item[i];
	    if (widget_contains(v->w, x, y)) {
		//change z-order
		darray_remove_index(layout_list, i);
		darray_append(layout_list, v);

		state = state_drag_vertex;
		select_vertex(v);
		dragx = x;
		dragy = y;
		return;
	    }

	    //check if output port was clicked on
	    x0 = v->w->x + v->w->w + vd_edgew;
	    y0 = v->w->y + vd_porty;
	    if (is_contained(x, y, x0, y0, x0 + vd_portw - 1, y0 + vd_porth)) {
		state = state_drag_port;
		select_vertex(v);
		return;
	    }
	}
	select_nothing();
	draw_canvas();

    } else if (widget_contains(compan, x, y)) {
	//it might be a button
	int i;
	for (i=0; i<command_list->count; i++) {
	    button_ptr p = (button_ptr) command_list->item[i];
	    if (widget_contains(p->w, x, y)) {
		state = state_button_pushed;
		button_selection = p;
		return;
	    }
	}
    } else if (widget_contains(aux_rect, x, y)) {
	textbox_ptr tb;
	int i;

	for (i=0; i<aux_tblist->count; i++) {
	    tb = (textbox_ptr) aux_tblist->item[i];
	    if (widget_contains(tb->w, x, y)) {
		textbox_handlembdown(tb, button,
			x - tb->w->x, y - tb->w->y);
	    }
	}
	if (widget_contains(auxbutton->w, x, y)) {
	    state = state_button_pushed;
	    button_selection = auxbutton;
	    button_update(auxbutton);
	    return;
	}
    } else if (widget_contains(info_rect, x, y)) {
	if (widget_contains(keycheckbox->w, x, y)) {
	    checkbox_handle_mousebuttondown(keycheckbox, button,
		    x - keycheckbox->w->x, y - keycheckbox->w->y);
	    keyboardplayflag = keycheckbox->state;
	    if (keyboardplayflag) {
		SDL_EnableKeyRepeat(0, 0);
	    } else {
		SDL_EnableKeyRepeat(150, 50);
	    }
	} else if (widget_contains(recordb->w, x, y)) {
	    button_handle_mousebuttondown(recordb, button,
		    x - recordb->w->x, y - recordb->w->y);
	} else if (widget_contains(playb->w, x, y)) {
	    button_handle_mousebuttondown(playb, button,
		    x - playb->w->x, y - playb->w->y);
	}
    }
}

static void root_handle_mousebuttonup(widget_ptr ignore, int button, int x, int y)
{
    if (state == state_menu) {
	state = state_normal;
	if (menu_selection->under) {
	    SDL_BlitSurface(menu_selection->under, NULL, screen, menu_selection->under_r);
	    SDL_FreeSurface(menu_selection->under);
	    menu_selection->under = NULL;
	}
	draw_menu();
	if (menu_selection->menu) {
	    darray_ptr m = menu_selection->menu;
	    int i;

	    if (widget_contains(menu_selection->w, x, y)) {
		menu_item_ptr it;
		i = (y - menu_selection->w->y - 2) / md_h; //2 for the border
		it = (menu_item_ptr) m->item[i];
		it->callback(it->data);
		return;
	    }
	}
    } else if (state == state_drag_vertex) {
	if (widget_contains(canvas, x, y)) {
	    widget_translate(vertex_selection->w, x - dragx, y - dragy);
	}
	state = state_normal;
	draw_canvas();
    } else if (state == state_drag_port) {
	int i, j, n;

	state = state_normal;

	for (i=layout_list->count-1; i>=0; i--) {
	    int x0, y0;
	    vertex_ptr v = (vertex_ptr) layout_list->item[i];
	    gen_ptr g = ((ins_node_ptr) v->node->data)->gen;
	    n = g->info->port_count;
	    y0 = v->w->y + vd_h + vd_porty;
	    for (j=0; j<n; j++) {
		x0 = v->w->x - vd_edgew - vd_portw;
		if (is_contained(x, y, x0 - vd_fudge, y0 - vd_fudge,
			    x0 + vd_portw - 1 + vd_fudge,
			    y0 + vd_porth - 1 + vd_fudge)) {
		    select_connection(add_connection(vertex_selection, v, j));
		    draw_canvas();
		    return;
		}
		y0 += vd_h;
	    }
	}
	draw_canvas();
    } else if (state == state_button_pushed) {
	state = state_normal;
	button_ptr p = button_selection;
	p->w->update(p->w);
	if (widget_contains(p->w, x, y)) {
	    p->callback(p->data);
	}
    }
}

static void root_handle_keydown(widget_ptr ignore, int sym, int mod)
{
    if (state == state_textbox) {
	textbox_handlekey(textbox_selection, sym, mod);
    }
}

static void aux_cancelprog(void *data)
{
}

static void aux_parseprog(void *data)
{
    textbox_ptr tb = (textbox_ptr) data;
    set_funk_program(vertex_selection->node, tb->s);
}

static void init_aux()
{
    int i;

    auxtb->cancel_cb = aux_cancelprog;
    auxtb->ok_cb = aux_parseprog;
    auxtb->ok_cb_data = (void *) auxtb;
    for (i=0; i<10; i++) {
	tbpool[i] = malloc(sizeof(textbox_t));
    }
    darray_init(aux_tblist);

    widget_init(aux_rect, root);
    textbox_init(auxtb, aux_rect);
    auxtb->w->localx = 5;
    auxtb->w->localy = 30;
    auxtb->w->w = aux_rect->w - 10;
    auxtb->w->h = 16;

    button_init(auxbutton, aux_rect);
    auxbutton->text = "Delete";
    auxbutton->img = SDL_LoadBMP("cancel.bmp");
    auxbutton->callback = delconcb;
    auxbutton->data = NULL;
    auxbutton->w->localx = 5;
    auxbutton->w->localy = 30;
    auxbutton->w->w = 32 + 4;
    auxbutton->w->h = 32 + 4;
}

static void track_clear()
{
    int i;
    for (i=0; i<track->count; i++) {
	free(track->item[i]);
    }
    darray_remove_all(track);
}

static void toggle_recordcb(void *data)
{
    if (is_recording) {
	is_recording = 0;
    } else {
	track_clear();
	is_recording = 1;
	last_tick = SDL_GetTicks();
    }
    draw_info();
}

static void track_add_event(int type, int x1, int x2)
{
    int t;
    event_ptr e;

    t = SDL_GetTicks();
    e = (event_ptr) malloc(sizeof(event_t));
    e->delta = t - last_tick;
    e->type = type;
    e->x1 = x1;
    e->x2 = x2;
    darray_append(track, e);
    last_tick = t;
}

static note_ptr keyboard[128];

static void midi_note_off(int noteno)
{
    if (keyboard[noteno]) {
	ins_note_off(keyboard[noteno]);
	keyboard[noteno] = NULL;
    }
    if (is_recording) track_add_event(ev_noteoff, noteno, 0);
}

static void midi_note_on(int noteno, int vel)
{
    SDL_LockAudio();
    if (keyboard[noteno]) {
	midi_note_off(noteno);
    }
    keyboard[noteno] = ins_note_on(ins, noteno, ((double) vel) / 127.0);
    SDL_UnlockAudio();
    if (is_recording) track_add_event(ev_noteon, noteno, vel);
}

static void do_event(event_ptr e)
{
    switch(e->type) {
	case ev_noteon:
	    midi_note_on(e->x1, e->x2);
	    break;
	case ev_noteoff:
	    midi_note_off(e->x1);
	    break;
    }
}

static Uint32 play_thread(Uint32 ignore)
{
    for (;;) {
	event_ptr e;
	if (!is_playing) return 0;
	e = track->item[play_i];
	do_event(e);
	play_i++;
	if (play_i >= track->count) {
	    is_playing = 0;
	    play_i = 0;
	    return 0;
	}
	e = track->item[play_i];
	if (e->delta) {
	    SDL_SetTimer(e->delta, play_thread);
	    return 0;
	}
    }
}

static void toggle_playcb(void *data)
{
    if (is_recording) {
	is_recording = 0;
	return;
    }
    if (is_playing) {
	is_playing = 0;
	//TODO: all notes off
    } else {
	if (track->count) {
	    is_playing = 1;
	    play_i = 0;
	    SDL_SetTimer(((event_ptr) track->item[0])->delta, play_thread);
	}
    }
}

static void init_info()
{
    widget_init(info_rect, root);

    checkbox_init(keycheckbox, info_rect);
    keycheckbox->w->localx = 5;
    keycheckbox->w->localy = 5;
    keycheckbox->state = 0;
    widget_show(keycheckbox->w);

    button_init(recordb, info_rect);
    recordb->w->localx = 5;
    recordb->w->localy = 50;
    button_make_text_image(recordb, "Record");
    widget_show(recordb->w);
    recordb->callback = toggle_recordcb;

    button_init(playb, info_rect);
    playb->w->localx = 5;
    playb->w->localy = 70;
    button_make_text_image(playb, "Play/Stop");
    widget_show(playb->w);
    playb->callback = toggle_playcb;
}

static void root_update()
{
    SDL_FillRect(screen, NULL, 0);
    draw_canvas();
    draw_command_panel();
    draw_info();
    draw_aux();
    draw_menu();
}

static double ticker()
{
    return ins_tick(ins);
}

static int symtonote[256];

static void specialkeydown(int sym, int mod)
{
    int n = symtonote[sym];
    if (n >=0) midi_note_on(n + 60, 64);
}

static void specialkeyup(int sym, int mod)
{
    int n = symtonote[sym];
    if (n >=0) midi_note_off(n + 60);
}

static void main_loop(void)
{
    while (state != state_quit && !interrupted) {
	widget_ptr w;
	SDL_Event event_;
	SDL_Event *event = &event_;

	SDL_GetMouseState(&lastmousex, &lastmousey);

	if (!displaywindow) {
	    w = root;

	    if (widget_contains(compan, lastmousex, lastmousey)) {
		draw_command_panel();
	    }
	    if (to_place && state == state_normal) {
		draw_placement();
	    }

	    switch (state) {
		case state_menu:
		    draw_menu();
		    break;
		case state_drag_vertex:
		case state_drag_port:
		    draw_canvas();
		    break;
	    }
	} else {
	    w = displaywindow->w;
	}

	SDL_Flip(screen);
	while (SDL_PollEvent(event)) switch (event->type) {
	    case SDL_QUIT:
		bliss_quit();
		break;
	    case SDL_VIDEORESIZE:
		main_resize(event->resize.w, event->resize.h);
		root->update(root);
		break;
	    case SDL_MOUSEBUTTONDOWN:
		if (widget_contains(w, event->button.x, event->button.y)) {
		    w->handle_mousebuttondown(w, event->button.button,
			    event->button.x, event->button.y);
		}
		break;
	    case SDL_MOUSEBUTTONUP:
		root->handle_mousebuttonup(root, event->button.button,
			event->button.x, event->button.y);
		break;
	    case SDL_KEYDOWN:
		if (keyboardplayflag) {
		    specialkeydown(event->key.keysym.sym,
			    event->key.keysym.mod);
		} else {
		    root->handle_keydown(root, event->key.keysym.sym,
			    event->key.keysym.mod);
		}
		break;
	    case SDL_KEYUP:
		if (keyboardplayflag) {
		    specialkeyup(event->key.keysym.sym,
			    event->key.keysym.mod);
		}
		break;
	    default:
		break;
	}
	SDL_Delay(10);
    }
}

static void interrupt(int i)
{
    interrupted = 1;
}

static void init_root()
{
    root->w = 640;
    root->h = 480;
    root->localx = 0;
    root->localy = 0;
    root->x = 0;
    root->y = 0;
    root->handle_mousebuttondown = root_handle_mousebuttondown;
    root->handle_mousebuttonup = root_handle_mousebuttonup;
    root->handle_keydown = root_handle_keydown;
    root->update = root_update;
}

static void init_canvas()
{
    widget_init(canvas, root);
    conimg = image_new(25, 25);
    SDL_SetColorKey(conimg, SDL_SRCCOLORKEY, 0);
    filledCircleColor(conimg, 12, 12, 12, colourgfx[c_shadow]);
    filledCircleColor(conimg, 12, 12, 11, colourgfx[c_highlight]);
    filledCircleColor(conimg, 12, 12, 10, colourgfx[c_unit]);
}

int main(int argc, char **argv)
{
    struct midi_cb_s midicbp = {
	midi_note_on,
	midi_note_off
    };

    signal(SIGINT, interrupt);
    signal(SIGTERM, interrupt);

    init_libs();

    init_utable();
    darray_init(track);

    init_root();
    init_command();
    init_menu();
    init_aux();
    {
	int i;
	unsigned char *s = "zsxdcvgbhnjm,l.;/";
	unsigned char *s2 = "q2w3er5t6y7ui9o0p[=]";
	for (i=0; i<256; i++) {
	    symtonote[i] = -1;
	}
	for (i=0; i<strlen(s); i++) {
	    symtonote[s[i]] = i;
	}
	for (i=0; i<strlen(s2); i++) {
	    symtonote[s2[i]] = i + 12;
	}
    }
    init_info();
    init_canvas();

    about_init(about_window, root);
    file_window_init(file_window, root);

    put_size(root->w, root->h);

    ins_editor_new();

    audio_set_ticker(ticker);
    state = state_normal;
    SDL_PauseAudio(0);
    midi_start(&midicbp);
    root->update(root);
    main_loop();
    midi_stop();
    SDL_PauseAudio(1);
    
    //TODO: free everything
    return 0;
}
