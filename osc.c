#include <stdlib.h>
#include <math.h>

#include "gen.h"

struct osc_data_s {
    double (*oscfn)(gen_t g, double d);
};

typedef struct osc_data_s osc_data_t[1];
typedef struct osc_data_s *osc_data_ptr;

void *osc_note_on()
{
    double *res = (double *) malloc(sizeof(double));
    *res = 0.0;
    return (void *) res;
}

void osc_note_free(void *data)
{
    free(data);
}

static double sawfn(gen_t g, double d)
{
    return 2 * (d - 0.5);
}

static double pulsefn(gen_t g, double d)
{
    if (d < 0.5) return 1.0;
    else return -1.0;
}

static double sinfn(gen_t g, double d)
{
    double res;
    res = sin(d * 2.0 * M_PI);
    return res;
}

double osc_tick(gen_t g, gen_data_ptr gd, double *value)
{
    double res;
    osc_data_ptr p;
    double *dp;

    dp = (double *) gd->data;
    *dp = (*dp) + value[0] * inv_samprate;
    if (*dp > 1.0) *dp -= 1.0;
    //else if (*dp < 0.0) *dp += 1.0;
    p = (osc_data_ptr) g->data;
    res = p->oscfn(g, *dp);
    return res;
}

void osc_init(gen_ptr g)
{
    g->data = malloc(sizeof(osc_data_t));
}

void osc_clear(gen_ptr g)
{
    free(g->data);
}

void shape_cb(gen_ptr g, double d)
{
    int n = (int) d;
    osc_data_ptr p;
    p = (osc_data_ptr) g->data;

    switch(n) {
	case 0:
	    p->oscfn = sinfn;
	    break;
	case 1:
	    p->oscfn = sawfn;
	    break;
	case 2:
	    p->oscfn = pulsefn;
	    break;
    }
}

struct param_s param_shape = {
    "shape",
    0,
    shape_cb
};

static char *osc_port_list[] = { "freq" };
static param_ptr osc_param_list[] = { &param_shape };

struct gen_info_s osc_info = {
    "osc",
    "Oscillator",
    osc_init,
    osc_clear,
    osc_note_on,
    osc_note_free,
    osc_tick,
    1,
    osc_port_list,
    1,
    osc_param_list,
};
