#include "gui.h"

#include "button.h"
#include "textbox.h"
#include "label.h"
#include "gen.h"
#include "voice.h"

static node_ptr current_node;

struct double_widget_s {
    label_t l;
    textbox_t tb;
};
typedef struct double_widget_s double_widget_t[1];
typedef struct double_widget_s *double_widget_ptr;

static void param_double_set(void *index, char *s)
{
    //TODO: pass g to this fn too?
    gen_ptr g = ((node_data_ptr) current_node->data)->gen;
    int i = (int) index;
    double d;
    sscanf(s, "%lf", &d);
    assign_double(g, i, d);
}

static void param_string_set(void *index, char *s)
{
    //TODO: pass g to this fn too?
    gen_ptr g = ((node_data_ptr) current_node->data)->gen;
    int i = (int) index;
    assign_string(g, i, s);
}

static int draw_double_init(void **ptr, widget_ptr w, int y0, gen_ptr g, int i)
{
    char s[80];
    gen_info_ptr gi = g->info;
    *ptr = malloc(sizeof(double_widget_t));
    double_widget_ptr p = *ptr;
    label_init(p->l, w);
    textbox_init(p->tb, w);
    p->l->w->localx = 5;
    p->l->w->localy = y0 + 5;

    label_put_text(p->l, gi->param[i]->id);
    widget_show(p->l->w);

    widget_put_geometry(p->tb->w, w->w / 2, y0, 80, 16);
    sprintf(s, "%.3f", to_double(g->param[i]));
    textbox_put_string(p->tb, s);
    textbox_put_ok_callback(p->tb, param_double_set, (void *) i);
    widget_show(p->tb->w);
    return 20;
}

static void draw_double_clear(widget_ptr w, void *ptr)
{
    double_widget_ptr p = ptr;
    label_clear(p->l);
    textbox_clear(p->tb);
    free(p);
}

static int draw_string_init(void **ptr, widget_ptr w, int y0, gen_ptr g, int i)
{
    char s[80];
    gen_info_ptr gi = g->info;
    *ptr = malloc(sizeof(double_widget_t));
    double_widget_ptr p = *ptr;
    label_init(p->l, w);
    textbox_init(p->tb, w);
    p->l->w->localx = 5;
    p->l->w->localy = y0 + 5;

    label_put_text(p->l, gi->param[i]->id);
    widget_show(p->l->w);

    widget_put_geometry(p->tb->w, 5, y0 + 20, w->w - 10, 16);
    sprintf(s, "%s", ((char *) g->param[i]));
    textbox_put_string(p->tb, s);
    textbox_put_ok_callback(p->tb, param_string_set, (void *) i);
    widget_show(p->tb->w);
    return 40;
}

static void draw_string_clear(widget_ptr w, void *ptr)
{
    double_widget_ptr p = ptr;
    label_clear(p->l);
    textbox_clear(p->tb);
    free(p);
}

static int (*draw_init[param_count])(void **ptr, widget_ptr w, int y0,
	gen_ptr g, int i);
static void (*draw_clear[param_count])(widget_ptr w, void *ptr);

struct mem_entry_s {
    void (*clear)(widget_ptr w, void *data);
    void *data;
};

typedef struct mem_entry_s *mem_entry_ptr;
typedef struct mem_entry_s mem_entry_t[1];

static darray_t mempool;

static textbox_ptr tbpool[10];
static label_ptr lpool[10];
static label_t auxid, auxname;
static button_t auxdelb, auxenterb;

widget_t aux_rect;

void aux_put_geometry(int x, int y, int w, int h)
{
    aux_rect->w = w;
    aux_rect->localy = y;
    aux_rect->h = h;
    aux_rect->localx = x;

    widget_put_location(auxdelb->w, 8 + 4 * (32 + 4 + 4),
	    aux_rect->h - (32 + 4 + 4) - 28);
    widget_put_location(auxenterb->w, 8,
	    aux_rect->h - (32 + 4 + 4) - 28);
}

void aux_show_nothing()
{
    darray_remove_all(aux_rect->show_list);
    widget_update(aux_rect);
    request_update(aux_rect);
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

static void delconcb(void *data)
{
    canvas_remove_current_edge(canvas);
}

static void delnodecb(void *data)
{
    SDL_LockAudio();
    canvas_remove_current_node(canvas);
    SDL_UnlockAudio();
}

static void entervoicecb(void *data)
{
    gui_edit_voice(data);
}

static void enterinscb(void *data)
{
    gui_edit_ins(data);
}

void aux_show_node(node_ptr node)
{
    int y0;
    int i, n;
    char s[80];
    gen_ptr g;
    void clearit(void *data) {
	mem_entry_ptr p = data;
	p->clear(aux_rect, p->data);
	free(p);
    }

    darray_forall(mempool, clearit);
    darray_remove_all(mempool);

    current_node = node;
    node_data_ptr p = (node_data_ptr) node->data;
    label_ptr l;
    textbox_ptr tb;
    button_ptr b;

    widget_hide_all(aux_rect);
    widget_string(aux_rect, 5, 5, p->id, c_text);

    switch (p->type) {

	case node_type_unit:
	    label_put_text(auxname, p->gen->info->name);
	    widget_show(auxname->w);
	    g = p->gen;
	    n = g->info->param_count;
	    y0 = 20 + 5;
	    for (i=0; i<n; i++) {
		mem_entry_ptr mep = malloc(sizeof(mem_entry_t));
		int type = g->info->param[i]->type;
		darray_append(mempool, mep);
		mep->clear = draw_clear[type];
		y0 += draw_init[type](&mep->data, aux_rect, y0, g, i);
		//TODO: what if y0 > ymax?
	    }

	    b = auxdelb;
	    widget_show(b->w);
	    button_put_callback(b, delnodecb, (void *) node);
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
	    button_put_callback(b, entervoicecb, p->voice);

	    b = auxdelb;
	    widget_show(b->w);
	    button_put_callback(b, delnodecb, NULL);
	    break;
	case node_type_ins:
	    current_ins = p->ins;
	    b = auxenterb;
	    widget_show(b->w);
	    button_put_callback(b, enterinscb, p->ins);
    }
    widget_update(aux_rect);
    request_update(aux_rect);
}

void aux_show_edge(edge_ptr c)
{
    node_data_ptr p0, p1;
    char s[80];

    darray_remove_all(aux_rect->show_list);
    p0 = (node_data_ptr) c->src->data;
    p1 = (node_data_ptr) c->dst->data;
    snprintf(s, 80, "Edge: %s --> %s", p0->id, p1->id);
    button_put_callback(auxdelb, delconcb, NULL);
    label_put_text(auxname, s);
    widget_show(auxname->w);
    widget_show(auxdelb->w);
    widget_update(aux_rect);
    request_update(aux_rect);
}

static void aux_rect_update(widget_ptr w)
{
    widget_raised_background(w);
    widget_draw_children(w);
}

void aux_init(widget_ptr parent)
{
    int i;

    darray_init(mempool);

    draw_init[param_double] = draw_double_init;
    draw_clear[param_double] = draw_double_clear;
    draw_init[param_string] = draw_string_init;
    draw_clear[param_string] = draw_string_clear;

    for (i=0; i<10; i++) {
	tbpool[i] = textbox_new(aux_rect);
	lpool[i] = label_new(aux_rect);
    }

    widget_init(aux_rect, parent);

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
