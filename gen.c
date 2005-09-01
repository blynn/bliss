#include <stdlib.h>
#include "gen.h"

#define DEFSAMPRATE 44100.0
double samprate = DEFSAMPRATE;
double inv_samprate = 1.0 / DEFSAMPRATE;
double nyquist = DEFSAMPRATE / 2.0;
double inv_nyquist = 2.0 / DEFSAMPRATE;

typedef void *(*param_init_fn)();
typedef void (*param_clear_fn)(void *);

static param_init_fn param_init[param_count];
static param_clear_fn param_clear[param_count];

static void *param_double_init()
{
    double *data = malloc(sizeof(double));
    from_double(data, 0.0);
    return data;
}

static void param_double_clear(void *data)
{
    free(data);
}

static void *param_string_init()
{
    void *data = NULL;
    from_string(&data, "");
    return data;
}

static void param_string_clear(void *data)
{
    free(data);
}

void param_type_init()
{
    void set_param_fns(int type, param_init_fn fi, param_clear_fn fc) {
	param_init[type] = fi;
	param_clear[type] = fc;
    }

    set_param_fns(param_double, param_double_init, param_double_clear);
    set_param_fns(param_string, param_string_init, param_string_clear);
}

void gen_init(gen_t g, gen_info_t gi)
{
    param_ptr *p;
    int i, n = gi->param_count;

    g->param = malloc(sizeof(void *) * n);
    for (i=0; i<n; i++) {
	g->param[i] = param_init[gi->param[i]->type]();
    }
    g->info = gi;
    gi->init(g);

    p = gi->param;
}

gen_ptr gen_new(gen_info_t gi)
{
    gen_ptr res = malloc(sizeof(gen_t));
    gen_init(res, gi);
    return res;
}

void gen_clear(gen_t g)
{
    gen_info_ptr gi = g->info;
    int i, n = gi->param_count;
    for (i=0; i<n; i++) {
	param_clear[gi->param[i]->type](g->param[i]);
    }
    gi->clear(g);
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
