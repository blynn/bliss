#include <stdlib.h>
#include "gen.h"

#define DEFSAMPRATE 44100.0
double samprate = DEFSAMPRATE;
double inv_samprate = 1.0 / DEFSAMPRATE;
double nyquist = DEFSAMPRATE / 2.0;
double inv_nyquist = 2.0 / DEFSAMPRATE;

void gen_init(gen_t g, gen_info_t gi)
{
    int i;
    param_ptr *p;

    g->info = gi;
    gi->init(g);
    g->param = malloc(sizeof(double) * gi->param_count);

    p = gi->param;
    for (i=0; i<gi->param_count; i++) {
	g->param[i] = p[i]->init_val;
	p[i]->callback(g, p[i]->init_val);
    }
}

gen_ptr gen_new(gen_info_t gi)
{
    gen_ptr res = malloc(sizeof(gen_t));
    gen_init(res, gi);
    return res;
}

void gen_clear(gen_t g)
{
    g->info->clear(g);
    free(g->param);
}

void gen_free(gen_ptr g)
{
    gen_clear(g);
    free(g);
}

void *gen_note_on(gen_t g)
{
    //perform any required per note initializations
    return g->info->note_on();
}

void gen_note_free(gen_t g, void *data)
{
    g->info->note_free(data);
}

double gen_tick(gen_ptr g, gen_data_ptr gd, double *value)
{
    return g->info->tick(g, gd, value);
}
