#include <stdlib.h>
#include <math.h>

#include "gen.h"

struct osc_data_s {
    double (*oscfn)(gen_t g, double d);
};

typedef struct osc_data_s osc_data_t[1];
typedef struct osc_data_s *osc_data_ptr;

static void *osc_note_on()
{
    double *res = (double *) malloc(sizeof(double));
    *res = 0.0;
    return (void *) res;
}

static void osc_note_free(void *data)
{
    free(data);
}

//the waveform is positive in the first half
//and negative in the second half for consistency,
static double sawfn(gen_t g, double d)
{
    return 1.0 - 2.0 * d;
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

static double trifn(gen_t g, double d)
{
    if (d < 0.25) return d * 4.0;
    if (d < 0.75) return 2.0 - 4.0 * d;
    return 4.0 * (d - 1.0);
}

static double osc_tick(gen_t g, gen_data_ptr gd, double *value)
{
    double res;
    osc_data_ptr p;
    double phase;

    phase = *((double *) gd->data);
    phase += double_clip(value[0], 0.0, nyquist) * inv_samprate;
    if (phase > 1.0) phase -= 1.0;
    p = (osc_data_ptr) g->data;
    res = p->oscfn(g, phase);
    *((double *) gd->data) = phase;
    return res;
}

static void osc_init(gen_ptr g)
{
    g->data = malloc(sizeof(osc_data_t));
}

static void osc_clear(gen_ptr g)
{
    free(g->data);
}

static void waveform_cb(gen_ptr g, double d)
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
	case 3:
	    p->oscfn = trifn;
	    break;
    }
}

static struct param_s param_waveform = {
    "waveform",
    0,
    waveform_cb
};

static char *osc_port_list[] = { "freq" };
static param_ptr osc_param_list[] = { &param_waveform };

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
