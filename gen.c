#include <stdlib.h>
#include "gen.h"

void gen_init(gen_t g, gen_info_t gi)
{
    int i;
    param_ptr *p;

    g->note_off_tick = NULL;
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

void *gen_note_on(gen_t g)
{
    //perform any required per note initializations
    return g->info->note_on();
}

void gen_note_free(gen_t g, void *data)
{
    g->info->note_free(data);
}

double gen_tick(gen_ptr g, void *data, double *value)
{
    return g->info->tick(g, data, value);
}

double gen_note_off_tick(gen_t g, void *data, double *value, int *dead)
{
    return g->note_off_tick(g, data, value, dead);
}
