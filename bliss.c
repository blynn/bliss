#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <SDL.h>

#include "audio.h"
#include "colour.h"
#include "voice.h"
#include "midi.h"

#include "textbox.h"
#include "menu.h"
#include "button.h"
#include "checkbox.h"
#include "window.h"
#include "layout.h"
#include "ins.h"

#include "about.h"
#include "file_window.h"
#include "version.h"

#include "htable.h"

#define SQRT3 1.732051

image_ptr conimg;

int lastmousex, lastmousey;

htable_t vltab;
layout_t ilp;
layout_ptr layout;

ins_t ins;

//TODO: hash table for this
vertex_ptr vertex_with_id(layout_ptr lp, char *id)
{
    int i;
    for (i=0; i<lp->vertex_list->count; i++) {
	vertex_ptr v = (vertex_ptr) lp->vertex_list->item[i];
	node_data_ptr p = (node_data_ptr) v->node->data;
	if (!strcmp(p->id, id)) {
	    return v;
	}
    }
    return NULL;
}

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

enum {
    place_unit = 1,
    place_voice,
};

static struct {
    int type;
    image_ptr img, old;
    rect_t r;
    int flag;
    uentry_ptr uentry;
} to_place;

extern gen_info_t clipper_info;
extern gen_info_t noise_info;
extern gen_info_t seg_info;
extern gen_info_t adsr_info;
extern gen_info_t osc_info;
extern gen_info_t out_info;
extern gen_info_t lpf_info;
extern gen_info_t butterhpf_info;
extern gen_info_t onezero_info;
extern gen_info_t onepole_info;
extern gen_info_t twopole_info;
extern gen_info_t lp4pole_info;
extern gen_info_t dummy_info;
gen_info_ptr funk_info_table[8];
extern gen_info_ptr funk_info_n(int);

extern gen_info_t shepard_info;

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
    add_entry("shepard", shepard_info, SDL_LoadBMP("shepard.bmp"));
    add_entry("osc", osc_info, SDL_LoadBMP("sine.bmp"));
    add_entry("adsr", adsr_info, SDL_LoadBMP("adsr.bmp"));
    add_entry("seg", seg_info, SDL_LoadBMP("seg.bmp"));
    add_entry("f", funk_info_table[2], SDL_LoadBMP("fx.bmp"));
    add_entry("out", out_info, NULL);
    add_entry("dummy", dummy_info, NULL);
    add_entry("hpf", butterhpf_info, SDL_LoadBMP("hpf.bmp"));
    add_entry("1zero", onezero_info, SDL_LoadBMP("zero.bmp"));
    add_entry("1pole", onepole_info, SDL_LoadBMP("pole.bmp"));
    add_entry("2pole", twopole_info, SDL_LoadBMP("twopole.bmp"));
    add_entry("lpf", lpf_info, SDL_LoadBMP("lpf.bmp"));
    add_entry("lpf", lp4pole_info, SDL_LoadBMP("lpf.bmp"));
    add_entry("noise", noise_info, SDL_LoadBMP("noise.bmp"));
    add_entry("clip", clipper_info, SDL_LoadBMP("clipper.bmp"));
}

static SDL_Surface *screen;

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
	SDL_BlitSurface(p->img, NULL, screen, p->r);
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
    SDL_BlitSurface(screen, p->r, p->img, NULL);
}

static saved_rect_t srpotedge;

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

static int interrupted = 0;

//TODO: ought to use a union?
vertex_ptr vertex_selection = NULL;
connection_ptr connection_selection = NULL;
int dragx, dragy;

static widget_t canvas;

void unit_init(vertex_ptr v, node_ptr node, int x, int y)
{
    node_data_ptr p;
    int n;
    v->node = node;
    v->w->localx = x;
    v->w->localy = y;
    v->w->globalx = x + canvas->globalx;
    v->w->globaly = y + canvas->globaly;

    p = (node_data_ptr) node->data;
    if (p->type == node_type_normal || p->type == node_type_funk) {
	gen_ptr g = ((node_data_ptr) v->node->data)->gen;
	v->inportcount = n = g->info->port_count;
    } else {
	v->inportcount = n = 0;
    }

    v->w->w = vd_w + 4;
    v->w->h = vd_h * (n + 1) + 4;
}

static void add_vertex(layout_ptr lp, node_ptr node, int x, int y)
{
    vertex_ptr v = vertex_new(lp);
    darray_append(lp->vertex_list, v);
    unit_init(v, node, x, y);
}

static node_ptr node_from_voice(voice_ptr voice)
{
    node_ptr node;
    node_data_ptr p;

    node = (node_ptr) malloc(sizeof(node_t));
    node->data = malloc(sizeof(node_data_t));
    p = (node_data_ptr) node->data;
    p->type = node_type_voice;
    p->voice = voice;
    p->id = voice->id; //TODO: get rid of node->id? voice has it anyway

    return node;
}

static connection_ptr add_connection(layout_ptr lp,
	vertex_ptr src, vertex_ptr dst, int port)
{
    connection_ptr c = (connection_ptr) malloc(sizeof(connection_t));

    widget_init(c->w, canvas);
    c->src = src;
    c->dst = dst;

    c->edge = voice_connect((voice_ptr) lp->data, src->node, dst->node, port);
    darray_append(lp->connection_list, c);
    return c;
}

enum {
    minroot_w = 600,
    minroot_h = 400,
    defroot_w = 800,
    defroot_h = 600,
};

int state;

static widget_t root;
static widget_t compan;
static widget_t aux_rect;
static widget_t info_rect;
static widget_t navbar;

static menubar_t menubar;

label_t navlocation;
button_ptr navupb;

window_t about_window;
file_window_t file_window;
window_ptr displaywindow = NULL;

void open_window(window_ptr w)
{
    displaywindow = w;
    widget_update(w->w);
    request_update(w->w);
}

void close_window()
{
    widget_update(root);
    request_update(displaywindow->w);
    displaywindow = NULL;
}

static textbox_t auxtb;
static textbox_ptr tbpool[10];
static label_ptr lpool[10];
static label_t auxid, auxname;
static button_t auxdelb, auxenterb;

static darray_ptr command_list;

static void remove_connection(connection_ptr c)
{
    darray_remove(layout->connection_list, c);
    voice_disconnect((voice_ptr) layout->data, c->edge);
    free(c);
}

static void put_size(int w, int h)
{
    menubar->w->localx = 0;
    menubar->w->localy = 0;
    menubar->w->w = w;

    navbar->localx = 0;
    navbar->localy = menubar->w->h;
    navbar->w = w;
    navbar->h = 32;

    info_rect->w = w;
    info_rect->h = 32;
    info_rect->localx = 0;
    info_rect->localy = menubar->w->h + navbar->h;

    compan->w = 5 * 40 - 4 + 16;
    compan->h = 144;
    compan->localx = 0;
    compan->localy = h - compan->h;

    canvas->localx = compan->w;
    canvas->localy = info_rect->localy + info_rect->h;
    canvas->w = w - compan->w;
    canvas->h = h - canvas->localy;

    aux_rect->w = compan->w;
    aux_rect->localy = info_rect->localy + info_rect->h;
    aux_rect->h = compan->localy - aux_rect->localy;
    aux_rect->localx = 0;

    auxtb->w->w = aux_rect->w - 10;

    widget_put_location(auxdelb->w, 8 + 4 * (32 + 4 + 4),
	    aux_rect->h - (32 + 4 + 4) - 28);
    widget_put_location(auxenterb->w, 8,
	    aux_rect->h - (32 + 4 + 4) - 28);

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

    flag = SDL_RESIZABLE | SDL_HWSURFACE | SDL_DOUBLEBUF;
    screen = SDL_SetVideoMode(defroot_w, defroot_h, 0, flag);
    widget_set_screen(screen);

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

static void compan_update(widget_ptr ignore)
{
    int i;

    widget_raised_background(compan);
    for (i=0; i<command_list->count; i++) {
	button_ptr p = (button_ptr) command_list->item[i];
	widget_ptr w = p->w;
	widget_update(w);
	if (global_contains(w, lastmousex, lastmousey)) {
	    widget_rectangle(w, -1, -1,  w->w, w->h, c_select);
	    widget_string(compan, 10, compan->h - 12, p->text, c_text);
	}
    }
}

static darray_ptr mainlist, cancellist, inseditlist;

static void put_command_list(darray_ptr a)
{
    command_list = a;
    widget_update(compan);
    request_update(compan);
}

static void draw_placement(widget_ptr w, int x0, int y0, int x1, int y1, void *data)
{
    int x, y;

    if (to_place.flag) {
	SDL_BlitSurface(to_place.old, NULL, screen, to_place.r);
	to_place.flag = 0;
    }
    x = x1 - to_place.r->w / 2;
    y = y1 - to_place.r->h / 2;
    if (widget_contains(canvas, x1, y1)) {
	to_place.r->x = x + canvas->globalx;
	to_place.r->y = y + canvas->globaly;
	if (to_place.r->x < 0) to_place.r->x = 0;
	if (to_place.r->x + to_place.r->w >= root->w) {
	    to_place.r->x = root->w - to_place.r->w - 1;
	}
	SDL_BlitSurface(screen, to_place.r, to_place.old, NULL);
	widget_clip(canvas);
	widget_blit(canvas, x, y, to_place.img);
	//TODO: only need to update the img
	request_update(canvas);
	widget_unclip();
	to_place.flag = 1;
    }
}

static void prepare_to_place(char *s)
{
    widget_string(canvas, canvas->w / 2 - 40, canvas->h - 20,
	    s, c_emphasis);

    to_place.img = image_new(to_place.r->w, to_place.r->h);
    to_place.old = image_new(to_place.r->w, to_place.r->h);
    image_box_rect(to_place.img, c_darkunit);
    widget_bind_mouse_motion(canvas, draw_placement, NULL);
}

static void prepare_to_place_unit(uentry_ptr u)
{
    to_place.type = place_unit;
    to_place.uentry = u;

    to_place.r->w = vd_w;
    to_place.r->h = (to_place.uentry->info->port_count + 1) * vd_h;

    prepare_to_place("Place Unit");
}

static void prepare_to_place_voice()
{
    to_place.type = place_voice;

    to_place.r->w = vd_w;
    to_place.r->h = vd_h;

    prepare_to_place("Place Voice");
}

static void done_to_place()
{
    to_place.type = 0;
    if (to_place.flag) {
	SDL_BlitSurface(to_place.old, NULL, screen, to_place.r);
	to_place.flag = 0;
    }
    image_free(to_place.img);
    image_free(to_place.old);
    widget_unbind_mouse_motion(canvas);
}

static void new_unit(void *data)
{
    prepare_to_place_unit((uentry_ptr) data);
    put_command_list(cancellist);
}

static void cancelbuttoncb(void *data)
{
    done_to_place();
    put_command_list(mainlist);
}

static void newvoicecb(void *data)
{
    prepare_to_place_voice();
    put_command_list(cancellist);
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

static void compan_handle_mousebuttondown(widget_ptr w,
	int button, int x, int y)
{
    int i;
    for (i=0; i<command_list->count; i++) {
	button_ptr p = (button_ptr) command_list->item[i];
	if (local_contains(p->w, x, y)) {
	    p->w->handle_mousebuttondown(p->w, button, x - p->w->localx, y - p->w->localy);
	    return;
	}
    }
}

static void compan_motion(widget_ptr w,
	int x0, int y0, int x1, int y1,
	void *data)
{
    widget_update(compan);
    request_update(compan);
}

static void init_command()
{
    button_ptr b;

    void button_from_unit(button_ptr b, char *id)
    {
	uentry_ptr p = utable_at(id);
	b->img = p->img;
	b->text = p->info->name;
	b->callback = new_unit;
	b->data = (void *) p;
    }

    widget_init(compan, root);
    widget_show(compan);
    compan->update = compan_update;
    compan->handle_mousebuttondown = compan_handle_mousebuttondown;
    widget_bind_mouse_motion(compan, compan_motion, NULL);

    command_list = mainlist = darray_new();
    b = new_command_button(0, 0);
    button_from_unit(b, "osc");
    darray_append(mainlist, b);

    b = new_command_button(0, 1);
    button_from_unit(b, "funk2");
    darray_append(mainlist, b);

    b = new_command_button(0, 2);
    button_from_unit(b, "adsr");
    darray_append(mainlist, b);

    b = new_command_button(0, 3);
    button_from_unit(b, "seg");
    darray_append(mainlist, b);

    b = new_command_button(0, 4);
    button_from_unit(b, "blop4plpf");
    darray_append(mainlist, b);

    b = new_command_button(2, 0);
    button_from_unit(b, "onezero");
    darray_append(mainlist, b);

    b = new_command_button(2, 1);
    button_from_unit(b, "onepole");
    darray_append(mainlist, b);

    b = new_command_button(2, 2);
    button_from_unit(b, "twopole");
    darray_append(mainlist, b);

    b = new_command_button(1, 3);
    button_from_unit(b, "clipper");
    darray_append(mainlist, b);

    b = new_command_button(1, 1);
    button_from_unit(b, "butterlpf");
    darray_append(mainlist, b);

    b = new_command_button(1, 2);
    button_from_unit(b, "butterhpf");
    darray_append(mainlist, b);

    b = new_command_button(1, 0);
    button_from_unit(b, "noise");
    darray_append(mainlist, b);

    b = new_command_button(1, 4);
    button_from_unit(b, "shepard");
    darray_append(mainlist, b);

    cancellist = darray_new();
    b = new_command_button(2, 4);
    b->text = "Cancel";
    b->img = SDL_LoadBMP("cancel.bmp");
    b->callback = cancelbuttoncb;
    b->data = NULL;
    darray_append(cancellist, b);

    inseditlist = darray_new();
    b = new_command_button(1, 0);
    b->text = "New Voice";
    b->img = SDL_LoadBMP("voice.bmp");
    b->callback = newvoicecb;
    b->data = NULL;
    darray_append(inseditlist, b);
}

static void draw_vertex(void *data)
{
    vertex_ptr v = data;
    int x0, x1, y0, y1;
    int i, n;
    node_data_ptr p = (node_data_ptr) v->node->data;
    widget_ptr w0 = v->w;

    //draw box containing unit
    widget_raised_border(w0);
    widget_box(w0, 2, 2, w0->w - 3, w0->h - 3, c_unit);

    if (p->type == node_type_funk || p->type == node_type_normal) {
	gen_ptr g = p->gen;

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
	widget_string(w0, 4, 4, p->id, c_emphasis);
    } else {
	widget_string(w0, 4, 4, p->voice->id, c_emphasis);
    }

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

static void draw_potentialedge(widget_ptr w,
	int xignore, int yignore, int x, int y,
	void *data)
{
    int x0, y0;
    int x1, y1;
    vertex_ptr v = vertex_selection;
    rect_t r;

    if (!widget_contains(w, x, y)) {
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

static void canvas_update(widget_ptr wcanvas)
{
    widget_clip(canvas);
    widget_box_rect(canvas, c_canvas);

    //draw vertices and connections
    darray_forall(layout->vertex_list, draw_vertex);
    darray_forall(layout->connection_list, draw_connection);

    draw_selectioncursor();
    widget_unclip(canvas);
}

static checkbox_t keycheckbox;
static label_t keycheckboxl;
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

static void info_rect_update(widget_ptr w)
{
    widget_raised_background(info_rect);
    widget_draw_children(info_rect);

    widget_filled_circle(info_rect, 175, 15, 8, c_darkshadow);
}

static void param_set(void *data, char *s)
{
    double d;
    int i = (int) data;
    node_ptr node = vertex_selection->node;
    sscanf(s, "%lf", &d);
    set_param(node, i, d);
}

static void select_nothing()
{
    connection_selection = NULL;
    vertex_selection = NULL;
    darray_remove_all(aux_rect->show_list);
    widget_update(aux_rect);
    request_update(aux_rect);
}

static void select_connection(connection_ptr c)
{
    vertex_selection = NULL;
    connection_selection = c;
    node_data_ptr p0, p1;
    char s[80];

    darray_remove_all(aux_rect->show_list);
    p0 = (node_data_ptr) c->src->node->data;
    p1 = (node_data_ptr) c->dst->node->data;
    snprintf(s, 80, "Edge: %s --> %s", p0->id, p1->id);
    button_put_callback(auxdelb, delconcb, NULL);
    label_put_text(auxname, s);
    widget_show(auxname->w);
    widget_show(auxdelb->w);
    widget_update(auxdelb->w);
    widget_update(aux_rect);
    request_update(aux_rect);
    //TODO: only need to draw connection
    widget_update(canvas);
    request_update(canvas);
}

static void navigate_voice(void *data);
static void editvoicecb(void *data)
{
    layout_ptr lp = (layout_ptr) htable_at(vltab, data);
    navigate_voice(lp);
}

static void delvoicecb(void *data)
{
    vertex_ptr v = data;
    //TODO: free node
    node_data_ptr p = (node_data_ptr) v->node->data;
    layout_ptr lp = htable_at(vltab, p->voice);
    layout_free(lp);
    layout_remove_vertex(layout, v);
    htable_remove(vltab, p->voice);
    voice_free(p->voice);
    select_nothing();
    widget_update(canvas);
    request_update(canvas);
}

static void setnotemincb(void *data, char *s)
{
    voice_ptr voice = (voice_ptr) data;
    voice->notemin = atoi(s);
}

static void setnotemaxcb(void *data, char *s)
{
    voice_ptr voice = (voice_ptr) data;
    voice->notemax = atoi(s);
}

static void delunitcb(void *data)
{
    vertex_ptr v = data;
    printf("Not implemented yet!\n");
    /*
    layout_remove_vertex(layout, v);
    //TODO: free node, edges, gen
    select_nothing();
    widget_update(canvas);
    request_update(canvas);
    */
}

static void select_vertex(vertex_ptr v)
{
    connection_selection = NULL;
    vertex_selection = v;
    int i, n;
    char s[80];
    gen_ptr g;
    node_ptr node = v->node;
    node_data_ptr p = (node_data_ptr) node->data;
    label_ptr l;
    textbox_ptr tb;
    button_ptr b;

    darray_remove_all(aux_rect->show_list);
    widget_string(aux_rect, 5, 5, p->id, c_text);

    switch (p->type) {
	case node_type_funk:
	    label_put_text(auxname, "Function:");
	    widget_show(auxname->w);
	    textbox_put_string(auxtb, get_funk_program(node));
	    textbox_update(auxtb);
	    widget_show(auxtb->w);

	    b = auxdelb;
	    widget_show(b->w);
	    button_put_callback(b, delunitcb, (void *) v);
	    break;
	case node_type_normal:
	    label_put_text(auxname, p->gen->info->name);
	    widget_show(auxname->w);
	    g = p->gen;
	    n = g->info->param_count;
	    for (i=0; i<n; i++) {
		lpool[i]->w->localx = 5;
		lpool[i]->w->localy = i * 20 + 20 + 5;
		label_put_text(lpool[i], g->info->param[i]->id);
		widget_show(lpool[i]->w);
		widget_put_geometry(tbpool[i]->w,
		    aux_rect->w / 2, i * 20 + 20,
		    80, 16);
		sprintf(s, "%.3f", g->param[i]);
		textbox_put_string(tbpool[i], s);
		textbox_put_ok_callback(tbpool[i], param_set, (void *) i);
		widget_show(tbpool[i]->w);
	    }

	    b = auxdelb;
	    widget_show(b->w);
	    button_put_callback(b, delunitcb, (void *) v);
	    break;
	case node_type_voice:
	    //label_put_text(auxid, p->voice->id);
	    //widget_show(auxid->w);
	    label_put_text(auxname, "Voice");
	    widget_show(auxname->w);
	    l = lpool[0];
	    l->w->localx = 5;
	    l->w->localy = 40 + 5;
	    label_put_text(l, "Bottom Note");
	    widget_show(l->w);

	    l = lpool[1];
	    l->w->localx = 5;
	    l->w->localy = 60 + 5;
	    label_put_text(l, "Top Note");
	    widget_show(l->w);

	    tb = tbpool[0];
	    widget_put_geometry(tb->w, aux_rect->w / 2, 40, 80, 16);
	    sprintf(s, "%d", p->voice->notemin);
	    textbox_put_string(tb, s);
	    textbox_put_ok_callback(tb, setnotemincb, (void *) p->voice);
	    widget_show(tb->w);

	    tb = tbpool[1];
	    widget_put_geometry(tb->w, aux_rect->w / 2, 60, 80, 16);
	    sprintf(s, "%d", p->voice->notemax);
	    textbox_put_string(tb, s);
	    textbox_put_ok_callback(tb, setnotemaxcb, (void *) p->voice);
	    widget_show(tb->w);

	    b = auxenterb;
	    widget_show(b->w);
	    button_put_callback(b, editvoicecb, (void *) p->voice);

	    b = auxdelb;
	    widget_show(b->w);
	    button_put_callback(b, delvoicecb, (void *) v);
	    break;
    }
    widget_update(aux_rect);
    request_update(aux_rect);
    //TODO: just need to move the selection cursor
    widget_update(canvas);
    request_update(canvas);
}

static void delconcb(void *data)
{
    remove_connection(connection_selection);
    select_nothing();
    widget_update(canvas);
    request_update(canvas);
}

static void track_clear();
static void file_clear()
{
    void clear_vertex(void *data) {
	vertex_ptr v = data;
	node_data_ptr p = (node_data_ptr) v->node->data;
	voice_ptr voice = p->voice;
	layout_free(htable_at(vltab, voice));
    }

    //free layout information
    darray_forall(ilp->vertex_list, clear_vertex);
    layout_clear(ilp);
    htable_remove_all(vltab);
    track_clear();

    vertex_selection = NULL;
    connection_selection = NULL;

    //free instrument which also frees the voices
    ins_clear(ins);
}

static void navigate_ins(void *data)
{
    widget_hide(navupb->w);
    layout = ilp;
    select_nothing();
    put_command_list(inseditlist);
    label_put_text(navlocation, "Instrument View");
    widget_show(navlocation->w);
    widget_update(navbar);
    widget_update(canvas);
    widget_update(compan);
    request_update(root);
}

static void navigate_voice(void *data)
{
    layout_ptr lp = data;
    button_put_callback(navupb, navigate_ins, NULL);
    widget_show(navupb->w);
    put_command_list(mainlist);
    select_nothing();
    layout = (layout_ptr) data;
    label_put_text(navlocation, ((voice_ptr) lp->data)->id);
    widget_show(navlocation->w);
    widget_update(navbar);
    widget_update(canvas);
    widget_update(compan);
    request_update(root);
}

static void file_new()
{
    ins_init(ins);
    voice_ptr voice;
    layout_ptr lp;

    layout_init(ilp, canvas, (void *) ins);

    voice = ins_add_voice(ins, "voice0");
    lp = layout_new(canvas, (void *) voice);
    htable_put(vltab, lp, voice);
    add_vertex(lp, voice->out, canvas->w - 100, canvas->h / 2);
    add_vertex(lp, voice->freq, 5, canvas->h / 2);

    add_vertex(ilp, node_from_voice(voice), canvas->w / 2, canvas->h / 2);

    navigate_voice(lp);
}

static void write_vlp(FILE *fp, layout_ptr vlp)
{
    void write_vertex(void *data)
    {
	int j;
	vertex_ptr v = data;
	node_ptr node = v->node;
	node_data_ptr p = (node_data_ptr) node->data;
	gen_ptr g = p->gen;

	fprintf(fp, "    unit %s %s ", p->id, g->info->id);
	fprintf(fp, "%d %d\n", v->w->localx, v->w->localy);
	if (p->type == node_type_funk) {
	    fprintf(fp, "    setfn %s %s\n", p->id, get_funk_program(node));
	} else if (p->type == node_type_normal) {
	    for (j=0; j<g->info->param_count; j++) {
		fprintf(fp, "    set %s %s %f\n", p->id,
			g->info->param[j]->id, g->param[j]);
	    }
	}
    }

    void write_connection(void *data)
    {
	connection_ptr c = data;
	edge_ptr e = c->edge;
	node_data_ptr p0, p1;
	p0 = (node_data_ptr) e->src->data;
	p1 = (node_data_ptr) e->dst->data;
	fprintf(fp, "    connect %s %s %d\n", p0->id, p1->id, *((int *) e->data));
    }

    darray_forall(vlp->vertex_list, write_vertex);
    darray_forall(vlp->connection_list, write_connection);
}

static void savecb(void *data, char *s)
{
    close_window();
    {
	FILE *fp;

	void write_voice(void *data) {
	    vertex_ptr v = data;
	    node_data_ptr p = (node_data_ptr) v->node->data;
	    voice_ptr voice = p->voice;
	    layout_ptr vlp = htable_at(vltab, voice);
	
	    fprintf(fp, "voice %s %d %d %d %d {\n", voice->id,
		    voice->notemin, voice->notemax,
		    v->w->localx, v->w->localy);
	    write_vlp(fp, vlp);
	    fprintf(fp, "}\n");
	}

	void write_track_event(void *data) {
	    event_ptr e = data;
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

	fp = fopen(s, "wb");

	fprintf(fp, "bliss %s\n", VERSION_STRING);

	darray_forall(ilp->vertex_list, write_voice);

	fprintf(fp, "track {\n");

	darray_forall(track, write_track_event);

	fprintf(fp, "}\n");
	fclose(fp);
    }
}

static void loadcb(void *data, char *filename)
{
    //TODO: error handling
    close_window();
    SDL_PauseAudio(1);
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

	void read_voice(layout_ptr vlp, voice_ptr voice) {
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
		    node = voice_add_gen(voice, u->info, s1);
		    read_word();
		    x = atoi(s);
		    read_word();
		    y = atoi(s);
		    add_vertex(vlp, node, x, y);
		    if (!strcmp(s1, "out")) {
			voice->out = node;
		    } else if (!strcmp(s1, "freq")) {
			voice->freq = node;
		    }
		} else if (!strcmp(s, "set")) {
		    vertex_ptr v;
		    int param;
		    read_word();
		    v = vertex_with_id(vlp, s);
		    read_word();
		    param = no_of_param(v->node, s);
		    read_word();
		    set_param(v->node, param, atof(s));
		} else if (!strcmp(s, "setfn")) {
		    vertex_ptr v;
		    read_word();
		    v = vertex_with_id(vlp, s);
		    read_word();
		    set_funk_program(v->node, s);
		} else if (!strcmp(s, "connect")) {
		    vertex_ptr v0, v1;
		    int port;
		    read_word();
		    v0 = vertex_with_id(vlp, s);
		    read_word();
		    v1 = vertex_with_id(vlp, s);
		    read_word();
		    port = atoi(s);
		    add_connection(vlp, v0, v1, port);
		} else break;
	    }
	}

	fp = fopen(filename, "rb");
	read_word();
	//TODO: check it's "bliss"
	read_word();
	//TODO: check version

	file_clear();
	ins_init(ins);
	layout_init(ilp, canvas, (void *) ins);
	for (;;) {
	    read_word();
	    if (!strcmp(s, "voice")) {
		int x, y;
		layout_ptr vlp;
		voice_ptr voice;

		read_word();
		voice = ins_add_voice(ins, s);
		vlp = layout_new(canvas, (void *) voice);
		htable_put(vltab, vlp, voice);
		read_word();
		voice->notemin = atoi(s);
		read_word();
		voice->notemax = atoi(s);
		read_word();
		x = atoi(s);
		read_word();
		y = atoi(s);
		add_vertex(ilp, node_from_voice(voice), x, y);
		read_word();
		//TODO: check s is "{"
		read_voice(vlp, voice);
		//TODO: check s is "}"
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
	    } else break;
	}
	fclose(fp);
	navigate_ins(NULL);
	widget_update(canvas);
	widget_update(aux_rect);
    }
    SDL_PauseAudio(0);
}

static void savemenuitemcb(void *data)
{
    file_window_setup(file_window, "Save File", "Save", savecb);
    open_window(file_window->win);
}

static void newmenuitemcb(void *data)
{
    file_clear();
    file_new();
    widget_update(root);
}

static double ticker()
{
    return ins_tick(ins);
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
	n = ticker() * 4096;
	write2(n);
	write2(n);
	t++;
    }
nomore:
    for (i=2*samprate; i; i--) {
	n = ticker() * 4096;
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
    menu_ptr m;

    menubar_init(menubar, root);
    widget_show(menubar->w);

    m = menubar_add_button(menubar, "File");
    menu_add_command(m, "Open...", loadmenuitemcb, NULL);
    menu_add_command(m, "Save...", savemenuitemcb, NULL);
    menu_add_command(m, "New", newmenuitemcb, NULL);
    menu_add_command(m, "Render", rendermenuitemcb, NULL);
    menu_add_command(m, "Quit", quitcb, NULL);

    m = menubar_add_button(menubar, "Help");
    menu_add_command(m, "About...", aboutcb, NULL);
}

static void root_handle_mousebuttondown(widget_ptr w, int button, int x, int y)
{
    int i;

    if (button != 1) return;

    for (i=0; i<w->show_list->count; i++) {
	widget_ptr w1 = (widget_ptr) w->show_list->item[i];
	if (global_contains(w1, x, y)) {
	    w1->handle_mousebuttondown(w1, button, x - w1->globalx, y - w1->globaly);
	    return;
	}
    }
}

static void aux_parseprog(void *data, char *s)
{
    set_funk_program(vertex_selection->node, s);
}

static void aux_rect_update(widget_ptr w)
{
    widget_raised_background(w);
    widget_draw_children(w);
}

static void init_aux()
{
    int i;

    auxtb->ok_cb = aux_parseprog;
    for (i=0; i<10; i++) {
	tbpool[i] = textbox_new(aux_rect);
	lpool[i] = label_new(aux_rect);
    }

    widget_init(aux_rect, root);
    textbox_init(auxtb, aux_rect);
    auxtb->w->localx = 5;
    auxtb->w->localy = 30;
    auxtb->w->w = aux_rect->w - 10;
    auxtb->w->h = 16;

    label_init(auxid, aux_rect);
    //auxid->w->localx = 10;
    //auxid->w->localy = 5;
    label_init(auxname, aux_rect);
    auxname->w->localx = 10;
    auxname->w->localy = 5;

    button_init(auxenterb, aux_rect);
    button_init(auxdelb, aux_rect);
    auxenterb->text = "Edit";
    auxenterb->img = SDL_LoadBMP("enter.bmp");
    auxenterb->w->w = 32 + 4;
    auxenterb->w->h = 32 + 4;

    button_init(auxdelb, aux_rect);
    auxdelb->text = "Delete";
    auxdelb->img = SDL_LoadBMP("skull.bmp");
    auxdelb->w->w = 32 + 4;
    auxdelb->w->h = 32 + 4;

    widget_show(aux_rect);
    aux_rect->update = aux_rect_update;
}

static void track_clear()
{
    int i;
    for (i=0; i<track->count; i++) {
	free(track->item[i]);
    }
    darray_remove_all(track);
}

static void stop_recording()
{
    is_recording = 0;
    widget_filled_circle(info_rect, 175, 15, 8, c_darkshadow);
    //TODO: only need to update the LED
    request_update(info_rect);
}

static void start_recording()
{
    track_clear();
    is_recording = 1;
    widget_filled_circle(info_rect, 175, 15, 8, c_led);
    last_tick = SDL_GetTicks();
    request_update(info_rect);
}

static void toggle_recordcb(void *data)
{
    if (is_playing) {
	printf("can't record while playing\n");
	return;
    }
    if (is_recording) {
	stop_recording();
    } else {
	start_recording();
    }
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

static void midi_note_off(int noteno)
{
    ins_note_off(ins, noteno);
    if (is_recording) track_add_event(ev_noteoff, noteno, 0);
}

static void midi_note_on(int noteno, int vel)
{
    SDL_LockAudio();
    ins_note_on(ins, noteno, ((double) vel) / 127.0);
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
	stop_recording();
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

static int keyboardplayflag = 0;

static int symtonote[256];

static void specialkeyup(int sym, int mod)
{
    int n = symtonote[sym];
    if (n >=0) midi_note_off(n + 60);
}

static int keyboard_midi_on(widget_ptr w, int sym, int mod, void *data)
{
    int n;
    if (!(sym >=0 && sym < 256)) return 0;
    n = symtonote[sym];
    if (n >=0) midi_note_on(n + 60, 64);
    return 0;
}

static void keyboardcb(void *data, int state)
{
    keyboardplayflag = state;
    if (keyboardplayflag) {
	//TODO: checkbox who called should also be sent?
	widget_push_keydowncb(NULL, keyboard_midi_on, NULL);
	SDL_EnableKeyRepeat(0, 0);
    } else {
	widget_pop_keydowncb();
	SDL_EnableKeyRepeat(150, 50);
    }
}

static void init_info()
{
    widget_init(info_rect, root);

    button_init(playb, info_rect);
    playb->w->localx = 5;
    playb->w->localy = 10;
    button_make_text_image(playb, "Play/Stop");
    widget_show(playb->w);
    playb->callback = toggle_playcb;

    button_init(recordb, info_rect);
    recordb->w->localx = 100;
    recordb->w->localy = 10;
    button_make_text_image(recordb, "Record");
    widget_show(recordb->w);
    recordb->callback = toggle_recordcb;

    checkbox_init(keycheckbox, info_rect);
    keycheckbox->w->localx = 200;
    keycheckbox->w->localy = 10;
    keycheckbox->state = 0;
    keycheckbox->callback = keyboardcb;
    widget_show(keycheckbox->w);

    label_init(keycheckboxl, info_rect);
    keycheckboxl->w->localx = 220;
    keycheckboxl->w->localy = 10;
    keycheckboxl->text = "Keyboard Plays";
    widget_show(keycheckboxl->w);

    widget_show(info_rect);
    info_rect->update = info_rect_update;
}

static void root_update()
{
    int i;
    widget_ptr w = root;
    SDL_FillRect(screen, NULL, 0);
    for (i=0; i<w->show_list->count; i++) {
	widget_ptr p = w->show_list->item[i];
	p->update(p);
    }
}

static void main_loop(void)
{
    SDL_GetMouseState(&lastmousex, &lastmousey);
    while (state != state_quit && !interrupted) {
	widget_ptr w;
	SDL_Event event_;
	SDL_Event *event = &event_;

	if (!displaywindow) {
	    w = root;
	} else {
	    w = displaywindow->w;
	}

	while (SDL_PollEvent(event)) switch (event->type) {
	    case SDL_QUIT:
		bliss_quit();
		break;
	    case SDL_VIDEORESIZE:
		main_resize(event->resize.w, event->resize.h);
		widget_update(root);
		request_update(root);
		break;
	    case SDL_MOUSEBUTTONDOWN:
		//TODO: how to get rid of this?
		if (state == state_textbox) {
		    state = state_normal;
		    textbox_update(textbox_selection);
		    request_update(textbox_selection->w);
		    textbox_selection = NULL;
		    widget_pop_keydowncb();
		}

		if (global_contains(w, event->button.x, event->button.y)) {
		    w->handle_mousebuttondown(w, event->button.button,
			    event->button.x, event->button.y);
		}
		break;
	    case SDL_MOUSEBUTTONUP:
		if (event->button.button != 1) break;

		root_button_up(root, event->button.button,
			event->button.x, event->button.y);
		break;
	    case SDL_KEYDOWN:
		root_key_down(event->key.keysym.sym,
			event->key.keysym.mod);
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
	{
	    int newx, newy;
	    SDL_GetMouseState(&newx, &newy);
	    if (newx != lastmousex || newy != lastmousey) {
		root_mouse_motion(lastmousex, lastmousey, newx, newy);
		lastmousex = newx;
		lastmousey = newy;
	    }
	}

	request_process();
	SDL_Delay(10);
    }
}

static void interrupt(int i)
{
    interrupted = 1;
}

static void init_root()
{
    root->w = defroot_w;
    root->h = defroot_h;
    root->localx = 0;
    root->localy = 0;
    root->globalx = 0;
    root->globaly = 0;
    root->handle_mousebuttondown = root_handle_mousebuttondown;
    root->update = root_update;
}

static void try_connect(widget_ptr w, int button, int x, int y, void *data)
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
		select_connection(add_connection(layout, vertex_selection, v, j));
		widget_update(canvas);
		request_update(canvas);
		return;
	    }
	    y0 += vd_h;
	}
    }
    widget_update(canvas);
    request_update(canvas);
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

static void finish_drag(widget_ptr w, int button, int x, int y, void *data)
{
    widget_unbind_mouse_motion(canvas);
}

static void canvas_handle_mousebuttondown(widget_ptr w, int button, int x, int y)
{
    int i;
    char id[80];
    int vx = x - vd_w / 2;
    int vy = y - vd_h / 2;
    voice_ptr voice;
    layout_ptr lp;

    if (to_place.type) {
	switch (to_place.type) {
	    case place_unit:
		for (i=0;;i++) {
		    sprintf(id, "%s%d", to_place.uentry->namebase, i);
		    if (!vertex_with_id(layout, id)) break;
		}
		add_vertex(layout, voice_add_gen((voice_ptr) layout->data,
			    to_place.uentry->info, id), vx, vy);
		put_command_list(mainlist);
		break;
	    case place_voice:
		for (i=0;;i++) {
		    sprintf(id, "voice%d", i);
		    if (!vertex_with_id(layout, id)) break;
		}
//TODO: YUCK!
voice = ins_add_voice(ins, id);
lp = layout_new(canvas, (void *) voice);
htable_put(vltab, lp, voice);
add_vertex(lp, voice->out, canvas->w - 100, canvas->h / 2);
add_vertex(lp, voice->freq, 5, canvas->h / 2);
add_vertex(layout, node_from_voice(voice), vx, vy);
		put_command_list(inseditlist);
		break;
	}
	done_to_place();
	select_vertex(darray_last(layout->vertex_list));
	widget_update(canvas);
	request_update(canvas);
	return;
    }

    for (i=layout->connection_list->count-1; i>=0; i--) {
	connection_ptr c = (connection_ptr) layout->connection_list->item[i];
	if (local_contains(c->w, x, y)) {
	    select_connection(c);
	    widget_update(canvas);
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

	    select_vertex(v);
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
	    select_vertex(v);
	    return;
	}
    }
    select_nothing();
    widget_update(canvas);
    request_update(canvas);
}

static void init_canvas()
{
    widget_init(canvas, root);
    conimg = image_new(25, 25);
    SDL_SetColorKey(conimg, SDL_SRCCOLORKEY, 0);
    image_filled_circle(conimg, 12, 12, 12, c_shadow);
    image_filled_circle(conimg, 12, 12, 11, c_highlight);
    image_filled_circle(conimg, 12, 12, 10, c_unit);
    widget_show(canvas);
    canvas->update = canvas_update;
    canvas->handle_mousebuttondown = canvas_handle_mousebuttondown;
}

static void navbar_update(widget_ptr w)
{
    widget_raised_background(w);
    widget_draw_children(w);
}

static void init_navbar()
{
    widget_init(navbar, root);
    navbar->update = navbar_update;
    navupb = button_new(navbar);
    navupb->img = SDL_LoadBMP("up.bmp");
    label_init(navlocation, navbar);
    widget_put_location(navlocation->w, 64, 15);
    widget_put_geometry(navupb->w, 10, 6, 20, 20);
    widget_show(navbar);
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
    htable_init(vltab);

    widget_system_init();

    init_root();
    init_command();
    init_menu();
    init_aux();
    init_navbar();
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

    file_new();

    audio_set_ticker(ticker);
    state = state_normal;
    midi_start(&midicbp);
    widget_update(root);
    request_update(root);
    SDL_PauseAudio(0);
    SDL_PauseAudio(0);
    main_loop();
    midi_stop();
    SDL_PauseAudio(1);
    
    //TODO: free everything
    return 0;
}
