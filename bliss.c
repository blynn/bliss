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
#include "compan.h"
#include "canvas.h"

#include "htable.h"

#include "config.h"

#include "version.h"


enum {
    nav_voice,
    nav_ins,
};

int navtype;
void *navdata;
void (*navplacecb)(int x, int y);

htable_t vltab;
layout_t ilp;

ins_t ins;

//TODO: hash table for this?
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

extern gen_info_t out_info;
extern gen_info_t dummy_info;

extern gen_info_t osc_info;
extern gen_info_t shepard_info;
extern gen_info_t random_wave_info;
extern gen_info_t noise_info;
extern gen_info_t stomperosc_info;

extern gen_info_t adsr_info;
extern gen_info_t stomperenv_info;
extern gen_info_t seg_info;

extern gen_info_t butterlpf_info;
extern gen_info_t butterhpf_info;
extern gen_info_t onezero_info;
extern gen_info_t onepole_info;
extern gen_info_t twopole_info;
extern gen_info_t lp4pole_info;

extern gen_info_t clipper_info;
extern gen_info_t delay_info;

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
    add_entry("out", out_info, NULL);
    add_entry("dummy", dummy_info, NULL);

    add_entry("osc", osc_info, SDL_LoadBMP("sine.bmp"));
    add_entry("noise", noise_info, SDL_LoadBMP("noise.bmp"));
    add_entry("random", random_wave_info, SDL_LoadBMP("noise.bmp"));
    add_entry("shepard", shepard_info, SDL_LoadBMP("shepard.bmp"));

    add_entry("adsr", adsr_info, SDL_LoadBMP("adsr.bmp"));
    add_entry("env", stomperenv_info, SDL_LoadBMP("stomperenv.bmp"));
    add_entry("osc", stomperosc_info, SDL_LoadBMP("sine.bmp"));
    add_entry("seg", seg_info, SDL_LoadBMP("seg.bmp"));

    add_entry("lpf", butterlpf_info, SDL_LoadBMP("lpf.bmp"));
    add_entry("hpf", butterhpf_info, SDL_LoadBMP("hpf.bmp"));
    add_entry("lpf", lp4pole_info, SDL_LoadBMP("lpf.bmp"));
    add_entry("1zero", onezero_info, SDL_LoadBMP("zero.bmp"));
    add_entry("1pole", onepole_info, SDL_LoadBMP("pole.bmp"));
    add_entry("2pole", twopole_info, SDL_LoadBMP("twopole.bmp"));

    add_entry("f", funk_info_table[2], SDL_LoadBMP("fx.bmp"));
    add_entry("clip", clipper_info, SDL_LoadBMP("clipper.bmp"));
    add_entry("delay", delay_info, SDL_LoadBMP("delay.bmp"));
}

static SDL_Surface *screen;

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

static void vertex_update(widget_ptr w0)
{
    int x0, x1, y0, y1;
    int i, n;
    vertex_ptr v = (vertex_ptr) w0;
    node_data_ptr p = (node_data_ptr) v->node->data;

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

static vertex_ptr add_vertex(layout_ptr lp, node_ptr node, int x, int y)
{
    vertex_ptr v = vertex_new(lp);
    v->w->update = vertex_update;
    darray_append(lp->vertex_list, v);
    unit_init(v, node, x, y);
    return v;
}

static connection_ptr add_connection(layout_ptr lp,
	vertex_ptr src, vertex_ptr dst, int port)
{
    int *ip = malloc(sizeof(int));
    *ip = port;
    return layout_add_connection(lp, src, dst, ip);
}

enum {
    minroot_w = 600,
    minroot_h = 400,
    defroot_w = 800,
    defroot_h = 600,
};

enum {
    state_normal,
    state_quit
};

static int state;

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

static textbox_t auxtb;
static textbox_ptr tbpool[10];
static label_ptr lpool[10];
static label_t auxid, auxname;
static button_t auxdelb, auxenterb;

static darray_ptr command_list, inseditlist;
static darray_ptr cancellist;

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
    char *s;
    int n;

    status = SDL_Init(SDL_INIT_EVERYTHING);
    if (status) {
	fprintf(stderr, "init: SDL_Init failed: %s\n", SDL_GetError());
	exit(-1);
    }
    atexit(SDL_Quit);

    //font_init();

    s = config_at("latency");
    n = s ? atoi(s) : 512;
    audio_init(n);

    SDL_WM_SetCaption("Bliss", "Bliss");
    SDL_WM_SetIcon(SDL_LoadBMP("icon.bmp"), NULL);

    flag = SDL_RESIZABLE | SDL_HWSURFACE | SDL_DOUBLEBUF;
    screen = SDL_SetVideoMode(defroot_w, defroot_h, 0, flag);
    widget_set_screen(screen);

    colour_init(screen->format);

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

static void place_node(node_ptr node, layout_ptr lp, int x, int y)
{
    vertex_ptr v = add_vertex(lp, node, x, y);

    switch (navtype) {
	case nav_voice:
	    compan_put(compan, command_list);
	    break;
	case nav_ins:
	    compan_put(compan, inseditlist);
	    break;
    }

    canvas_select_vertex(canvas, v);
}

static void place_voice(void *data, layout_ptr lp, int x, int y)
{
    int i;
    char id[80];
    voice_ptr voice;
    node_ptr node;
    node_data_ptr p;
    layout_ptr lpv;

    for (i=0;;i++) {
	sprintf(id, "voice%d", i);
	if (!vertex_with_id(lp, id)) break;
    }
//TODO: YUCK!
    node = ins_add_voice(ins, id);
p = node->data;
voice = p->voice;
lpv = layout_new(canvas, voice->graph);
htable_put(vltab, lpv, voice);
add_vertex(lpv, voice->out, canvas->w - 100, canvas->h / 2);
add_vertex(lpv, voice->freq, 5, canvas->h / 2);
    place_node(node, lp, x, y);
}

static void place_unit(void *data, layout_ptr lp, int x, int y)
{
    int i;
    char id[80];
    uentry_ptr u = data;

    for (i=0;;i++) {
	sprintf(id, "%s%d", u->namebase, i);
	if (!vertex_with_id(lp, id)) break;
    }
    place_node(node_from_gen_info(lp->graph, u->info, id), lp, x, y);
}

static void prepare_to_place_unit(uentry_ptr u)
{
    canvas_placement_start(canvas, place_unit, u,
	    vd_w,
	    (u->info->port_count + 1) * vd_h,
	    "Place Unit");
}

static void prepare_to_place_voice()
{
    canvas_placement_start(canvas, place_voice, NULL,
	    vd_w, vd_h,
	    "Place Voice");
}

static void new_unit(void *data)
{
    prepare_to_place_unit((uentry_ptr) data);
    compan_push(compan, cancellist);
}

static void cancelbuttoncb(void *data)
{
    canvas_placement_finish(canvas);
    compan_pop(compan);
    //TODO: remove update "Place Unit" text
}

static void newvoicecb(void *data)
{
    prepare_to_place_voice();
    compan_push(compan, cancellist);
}

static void delconcb(void *data);

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
    node_ptr node = canvas_current_vertex(canvas)->node;
    sscanf(s, "%lf", &d);
    set_param(node, i, d);
}

static void select_nothing_cb()
{
    darray_remove_all(aux_rect->show_list);
    widget_update(aux_rect);
    request_update(aux_rect);
}

static void select_connection_cb(connection_ptr c)
{
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
    widget_update(aux_rect);
    request_update(aux_rect);
    //TODO: only need to draw connection
    widget_update(canvas);
    request_update(canvas);
}

static void navigate_voice(voice_ptr voice);
static void editvoicecb(void *data)
{
    navigate_voice((voice_ptr) data);
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

static void delvertexcb(void *data)
{
    SDL_LockAudio();
    canvas_remove_current_vertex(canvas);
    SDL_UnlockAudio();
}

static void delvoicevertexcb(void *data)
{
    vertex_ptr v = canvas_current_vertex(canvas);
    node_data_ptr p = (node_data_ptr) v->node->data;
    layout_ptr lp = htable_at(vltab, p->voice);
    SDL_LockAudio();
    layout_free(lp);
    htable_remove(vltab, p->voice);
    delvertexcb(NULL);
    SDL_UnlockAudio();
}

static void select_vertex_cb(vertex_ptr v)
{
    int i, n;
    char s[80];
    gen_ptr g;
    node_ptr node = v->node;
    node_data_ptr p = (node_data_ptr) node->data;
    label_ptr l;
    textbox_ptr tb;
    button_ptr b;

    widget_hide_all(aux_rect);
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
	    button_put_callback(b, delvertexcb, NULL);
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
	    button_put_callback(b, delvertexcb, (void *) v);
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
	    button_put_callback(b, delvoicevertexcb, NULL);
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
    canvas_remove_current_connection(canvas);
}

static void track_clear();
static void file_clear()
{
    void clear_vlp(void *data) {
	layout_free((layout_ptr) data);
    }

    SDL_LockAudio();
    htable_forall(vltab, clear_vlp);
    layout_clear(ilp);
    //TODO: stop playing track
    ins_clear(ins);
    track_clear();
    SDL_UnlockAudio();
    htable_remove_all(vltab);
}

static void navigate_ins(void *data)
{
    navtype = nav_ins;
    navdata = ins;
    widget_hide_all(navbar);
    canvas_put_layout(canvas, ilp);
    canvas_select_nothing(canvas);
    compan_put(compan, inseditlist);
    label_put_text(navlocation, "Instrument View");
    widget_show(navlocation->w);
    widget_update(navbar);
    widget_update(canvas);
    widget_update(compan);
    request_update(root);
}

static void navigate_voice(voice_ptr voice)
{
    navtype = nav_voice;
    navdata = voice;
    button_put_callback(navupb, navigate_ins, NULL);
    widget_hide_all(navbar);
    widget_show(navupb->w);
    compan_put(compan, command_list);
    canvas_put_layout(canvas, (layout_ptr) htable_at(vltab, voice));
    canvas_select_nothing(canvas);
    label_put_text(navlocation, voice->id);
    widget_show(navlocation->w);
    widget_update(navbar);
    widget_update(canvas);
    widget_update(compan);
    request_update(root);
}

static void file_new()
{
    voice_ptr voice;
    node_ptr node;
    node_data_ptr p;
    layout_ptr lp;
    vertex_ptr v0, v1;

    ins_init(ins, "ins0");
    layout_init(ilp, canvas, ins->graph);
    v1 = add_vertex(ilp, ins->out, canvas->w - 100, canvas->h / 2);

    node = ins_add_voice(ins, "voice0");
p = node->data;
voice = p->voice;
lp = layout_new(canvas, voice->graph);
htable_put(vltab, lp, voice);
add_vertex(lp, voice->out, canvas->w - 100, canvas->h / 2);
add_vertex(lp, voice->freq, 5, canvas->h / 2);

    v0 = add_vertex(ilp, node, 5, canvas->h / 2);
    add_connection(ilp, v0, v1, 0);

    navigate_voice(voice);
}

static void write_layout(FILE *fp, layout_ptr lp)
{
    void write_vertex(void *data)
    {
	int j;
	vertex_ptr v = data;
	node_ptr node = v->node;
	node_data_ptr p = (node_data_ptr) node->data;

	switch(p->type) {
	    gen_ptr g;
	    voice_ptr voice;
	    case node_type_normal:
	    case node_type_funk:
		g = p->gen;

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
		break;
	    case node_type_voice:
		voice = p->voice;
		layout_ptr vlp = htable_at(vltab, voice);
	    
		fprintf(fp, "voice %s %d %d %d %d {\n", voice->id,
			voice->notemin, voice->notemax,
			v->w->localx, v->w->localy);
		write_layout(fp, vlp);
		fprintf(fp, "}\n");
		break;
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

    darray_forall(lp->vertex_list, write_vertex);
    darray_forall(lp->connection_list, write_connection);
}

static void savecb(char *filename)
{
    {
	FILE *fp;

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

	fp = fopen(filename, "wb");

	fprintf(fp, "bliss %s\n", VERSION_STRING);

	//fprintf(fp, "ins %s {\n", ins->id);
	write_layout(fp, ilp);

	fprintf(fp, "track {\n");

	darray_forall(track, write_track_event);

	fprintf(fp, "}\n");

	//fprintf(fp, "}\n");
	fclose(fp);
    }
}

static voice_ptr voice_kludge;
static void read_layout(FILE *fp, layout_ptr lp)
{
    char s[256];
    char s1[256];
    void read_word() {
	int i;
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
	    if (!strcmp(s1, "out")) {
		if (voice_kludge) {
		    node = voice_kludge->out;
		} else  {
		    node = ins->out;
		}
	    } else if (!strcmp(s1, "freq")) {
		node = voice_kludge->freq;
	    } else {
		node = node_from_gen_info(lp->graph, u->info, s1);
	    }
	    read_word();
	    x = atoi(s);
	    read_word();
	    y = atoi(s);
	    add_vertex(lp, node, x, y);
	} else if (!strcmp(s, "set")) {
	    vertex_ptr v;
	    int param;
	    read_word();
	    v = vertex_with_id(lp, s);
	    read_word();
	    param = no_of_param(v->node, s);
	    if (param < 0) {
		printf("No such parameter: %s\n", s);
		read_word();
	    } else {
		read_word();
		set_param(v->node, param, atof(s));
	    }
	} else if (!strcmp(s, "setfn")) {
	    vertex_ptr v;
	    read_word();
	    v = vertex_with_id(lp, s);
	    read_word();
	    set_funk_program(v->node, s);
	} else if (!strcmp(s, "connect")) {
	    vertex_ptr v0, v1;
	    int port;
	    read_word();
	    v0 = vertex_with_id(lp, s);
	    read_word();
	    v1 = vertex_with_id(lp, s);
	    read_word();
	    port = atoi(s);
	    add_connection(lp, v0, v1, port);
	} else if (!strcmp(s, "voice")) {
	    int x, y;
	    layout_ptr vlp;
	    voice_ptr voice;
	    node_ptr node;
	    node_data_ptr p;

	    read_word();
	    node = ins_add_voice(ins, s);
	    p = node->data;
	    voice = p->voice;
	    vlp = layout_new(canvas, voice->graph);
	    htable_put(vltab, vlp, voice);
	    read_word();
	    voice->notemin = atoi(s);
	    read_word();
	    voice->notemax = atoi(s);
	    read_word();
	    x = atoi(s);
	    read_word();
	    y = atoi(s);
	    add_vertex(lp, node, x, y);
	    read_word();
	    //TODO: check s is "{"
	    voice_kludge = voice;
	    read_layout(fp, vlp);
	    voice_kludge = NULL;
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
}

static void loadcb(char *filename)
{
    //TODO: error handling
    SDL_PauseAudio(1);
    {
	FILE *fp;
	char s[256];

	void read_word() {
	    int i;
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

	file_clear();
	ins_init(ins, "ins0");
	layout_init(ilp, canvas, ins->graph);
	read_layout(fp, ilp);
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
    window_open(file_window->win);
}

static void newmenuitemcb(void *data)
{
    file_clear();
    file_new();
    widget_update(root);
    request_update(root);
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
    window_open(file_window->win);
}

static void quitcb(void *data)
{
    bliss_quit();
}

static void aboutcb(void *data)
{
    window_open(about_window);
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

static void aux_parseprog(void *data, char *s)
{
    set_funk_program(canvas_current_vertex(canvas)->node, s);
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

static Uint32 play_thread(Uint32 ignore, void *data)
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
	    return e->delta;
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
	    SDL_AddTimer(((event_ptr) track->item[0])->delta, play_thread, NULL);
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
    } else {
	widget_pop_keydowncb();
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
    int lastmousex, lastmousey;
    int newx = 0, newy = 0; //to get rid of compiler warning
    int motiontimer = 0;

    Uint32 motioncb(Uint32 interval, void *data)
    {
	motion_notify();
	return 0;
    }

    SDL_GetMouseState(&lastmousex, &lastmousey);
    while (state != state_quit) {
	SDL_Event event_;
	SDL_Event *event = &event_;

	//TODO: error check
	SDL_WaitEvent(event);

	switch (event->type) {
	    case SDL_USEREVENT:
		switch(event->user.code) {
		    case code_motion:
			motiontimer = 0;
			SDL_GetMouseState(&newx, &newy);
			root_mouse_motion(lastmousex, lastmousey, newx, newy);
			lastmousex = newx;
			lastmousey = newy;
			break;
		    case code_interrupt:
			bliss_quit();
			break;
		}
		break;
	    case SDL_QUIT:
		bliss_quit();
		break;
	    case SDL_VIDEORESIZE:
		main_resize(event->resize.w, event->resize.h);
		widget_update(root);
		request_update(root);
		break;
	    case SDL_MOUSEBUTTONDOWN:
		/*
		if (!displaywindow) {
		    w = root;
		} else {
		    w = displaywindow->w;
		}
		*/
		root_button_down(root, event->button.button,
			event->button.x, event->button.y);
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
	    case SDL_MOUSEMOTION:
		if (!motiontimer) {
		    motiontimer = 1;
		    SDL_AddTimer(10, motioncb, NULL);
		}
		break;
	    default:
		break;
	}
    }
}

SDL_Event event_interrupt;

static void interrupt(int i)
{
    SDL_PushEvent(&event_interrupt);
}

static void init_root()
{
    root->w = defroot_w;
    root->h = defroot_h;
    root->localx = 0;
    root->localy = 0;
    root->globalx = 0;
    root->globaly = 0;
    root->handle_mousebuttondown = root_button_down;
    root->update = root_update;
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

static void buttonmenubackcb(void *data)
{
    compan_pop(compan);
}

static void openbuttonmenucb(void *data)
{
    compan_push(compan, (darray_ptr) data);
}

static void init_command()
{
    button_ptr b;
    darray_ptr l;
    image_ptr imgcancel = SDL_LoadBMP("cancel.bmp");

    void add_unit_button(darray_ptr target_list, int row, int col, char *id)
    {
	uentry_ptr p = utable_at(id);
	b = compan_new_button(compan, row, col);
	b->img = p->img;
	b->text = p->info->name;
	b->callback = new_unit;
	b->data = (void *) p;
	darray_append(target_list, b);
    }

    darray_ptr new_list_button(darray_ptr target_list, int row, int col,
	    char *text, image_ptr image) {
	darray_ptr res = darray_new();
	b = compan_new_button(compan, 2, 4);
	b->img = imgcancel;
	b->text = "Cancel";
	b->callback = buttonmenubackcb;
	b->data = NULL;
	darray_append(res, b);

	b = compan_new_button(compan, row, col);
	b->img = image;
	b->text = text;
	b->callback = openbuttonmenucb;
	b->data = (void *) res;
	darray_append(target_list, b);
	return res;
    }

    void add_button(darray_ptr target_list, int row, int col,
	    char *text, image_ptr image,
	    void (*callback)(void *), void *data) {
	b = compan_new_button(compan, row, col);
	b->text = text;
	b->img = image;
	b->callback = callback;
	b->data = data;
	darray_append(target_list, b);
    }

    compan_init(compan, root);

    command_list = darray_new();
    inseditlist = darray_new();

    l = new_list_button(command_list, 0, 0, "Oscillators", SDL_LoadBMP("sine.bmp"));
    add_unit_button(l, 0, 0, "osc");
    add_unit_button(l, 0, 1, "noise");
    add_unit_button(l, 0, 2, "bloprandomwave");
    add_unit_button(l, 0, 3, "stomperosc");
    add_unit_button(l, 1, 0, "shepard");


    l = new_list_button(command_list, 0, 1, "Envelopes", SDL_LoadBMP("adsr.bmp"));
    add_unit_button(l, 0, 0, "adsr");
    add_unit_button(l, 0, 1, "stomperenv");


    l = new_list_button(command_list, 0, 2, "Filters", SDL_LoadBMP("lpf.bmp"));
    add_unit_button(l, 0, 0, "butterlpf");
    add_unit_button(l, 0, 1, "butterhpf");
    add_unit_button(l, 1, 0, "blop4plpf");
    add_unit_button(l, 2, 0, "onezero");
    add_unit_button(l, 2, 1, "onepole");
    add_unit_button(l, 2, 2, "twopole");

    add_unit_button(command_list, 1, 0, "funk2");
    add_unit_button(command_list, 1, 1, "seg");
    add_unit_button(command_list, 1, 2, "clipper");

    add_unit_button(command_list, 1, 3, "delay");

    cancellist = darray_new();
    add_button(cancellist, 2, 4, "Cancel", imgcancel,
	    cancelbuttoncb, NULL);

    add_button(inseditlist, 2, 0, "New Voice", SDL_LoadBMP("voice.bmp"),
		    newvoicecb, NULL);
    darray_append(inseditlist, command_list->item[0]);
    darray_append(inseditlist, command_list->item[1]);
    darray_append(inseditlist, command_list->item[2]);
    darray_append(inseditlist, command_list->item[3]);
    darray_append(inseditlist, command_list->item[4]);
    darray_append(inseditlist, command_list->item[5]);
    darray_append(inseditlist, command_list->item[6]);
}

static void connect_vertex_cb(layout_ptr lp, vertex_ptr v0, vertex_ptr v1, int j)
{
    canvas_select_connection(canvas, add_connection(lp, v0, v1, j));
}

int main(int argc, char **argv)
{
    struct midi_cb_s midicbp = {
	midi_note_on,
	midi_note_off
    };

    event_interrupt.type = SDL_USEREVENT;
    event_interrupt.user.code = code_interrupt;
    signal(SIGINT, interrupt);
    signal(SIGTERM, interrupt);

    config_init();

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
    canvas_init(canvas, root,
	    select_nothing_cb,
	    select_connection_cb,
	    select_vertex_cb,
	    connect_vertex_cb);

    about_init(about_window, root);
    file_window_init(file_window, root);

    put_size(root->w, root->h);

    file_new();

    audio_set_ticker(ticker);
    state = state_normal;
    midi_start(&midicbp);
    widget_update(root);
    request_update(root);
    audio_start();
    main_loop();
    midi_stop();
    audio_stop();
    
    //TODO: free everything
    return 0;
}
