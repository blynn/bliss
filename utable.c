#include "button.h"
#include "voice.h"

#include "utable.h"

darray_t utable;

uentry_ptr out_uentry;

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

static void standard_control(widget_ptr w, node_ptr node)
{
    /*
    node_data_ptr p = node->data;
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
    */
}

void utable_init()
{
    int i;
    darray_init(utable);
    for (i=0; i<8; i++) {
	funk_info_table[i] = funk_info_n(i);
    }

    uentry_ptr add_entry(char *namebase, gen_info_ptr info, image_ptr img)
    {
	uentry_ptr p = (uentry_ptr) malloc(sizeof(uentry_t));
	p->namebase = namebase;
	p->info = info;
	p->img = img;
	p->draw_control = standard_control;
	darray_append(utable, p);
	return p;
    }
    out_uentry = add_entry("out", out_info, NULL);
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

void utable_clear()
{
    void free_entry(void *data)
    {
	uentry_ptr p = (uentry_ptr) data;
	if (p->img) SDL_FreeSurface(p->img);
	free(data);
    }
    darray_forall(utable, free_entry);
    darray_clear(utable);
}
