//Stomper Hyperion oscillator clone
//Warning: effect of "Noise Type" depends on sampling rate
//I might change this
#include <stdlib.h>
#include <math.h>

#include "gen.h"

#define LOG2OVERLOG100 0.150515

static inline double randminus1to1()
{
    return rand() * (2.0 / (double) RAND_MAX) - 1.0;
}

static inline int sign(double d)
{
    if (d > 0.0) return 1;
    if (d < 0.0) return -1;
    return 0;
}

struct stomperosc_data_s {
    double (*oscfn)(gen_t g, double d);
    double exp, duty;
    double noisefactor;
    int noisetype;
};

typedef struct stomperosc_data_s stomperosc_data_t[1];
typedef struct stomperosc_data_s *stomperosc_data_ptr;

struct stomperosc_notedata_s {
    int noisecounter;
    double noisevalue;
    double last;
    double phase;
};

typedef struct stomperosc_notedata_s stomperosc_notedata_t[1];
typedef struct stomperosc_notedata_s *stomperosc_notedata_ptr;

static void *stomperosc_note_on()
{
    stomperosc_notedata_ptr np = (stomperosc_notedata_ptr) malloc(sizeof(stomperosc_notedata_t));
    np->noisecounter = 0;
    np->noisevalue = 0.0;
    np->phase = 0.0;
    np->last = 0.0;
    return (void *) np;
}

static void stomperosc_note_free(void *data)
{
    free(data);
}

//Same functions as the ones in osc.c
static double sawfn(gen_t g, double d)
{
    return 1.0 - 2.0 * d;
}

static double pulsefn(gen_t g, double d)
{
    stomperosc_data_ptr p = g->data;
    if (d < p->duty) return 1.0;
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

static double stomperosc_tick(gen_t g, gen_data_ptr gd, double *value)
{
    double res;
    stomperosc_data_ptr p = g->data;
    stomperosc_notedata_ptr np = gd->data;
    double f = value[0];

    if (p->noisefactor) {
	if (p->noisetype) {
	    if (!np->noisecounter) {
		np->noisevalue = randminus1to1();
	    }
	    np->noisecounter++;
	    if (np->noisecounter >= p->noisetype) np->noisecounter = 0;
	}

	f *= p->noisefactor * np->noisevalue + 1.0;
    }

    np->phase += double_clip(f, 0.0, nyquist) * inv_samprate;
    if (np->phase > 1.0) np->phase -= 1.0;

    p = (stomperosc_data_ptr) g->data;
//Stomper's square wave is positive and then negative
//The other three have negative then positive
    res = -p->oscfn(g, np->phase);
    if (p->oscfn == pulsefn) {
	res *= value[1];
    } else {
	if (res > 0.0) {
//...which is the reason the signs are the other way around here
	    res = -pow(res, p->exp) * value[1];
	} else {
	    res = pow(fabs(res), p->exp) * value[1];
	}
    }

    if (p->noisefactor && !p->noisetype) {
	if (sign(np->last) != sign(res)) {
	    np->noisevalue = randminus1to1();
	}
	np->last = res;
    }
    return res;
}

static void stomperosc_init(gen_ptr g)
{
    g->data = malloc(sizeof(stomperosc_data_t));
    assign_double(g, 1, 1.0); //set shape to 1.0
}

static void stomperosc_clear(gen_ptr g)
{
    free(g->data);
}

static void shape_cb(gen_ptr g, void *data)
{
    stomperosc_data_ptr p;
    p = (stomperosc_data_ptr) g->data;
    p->exp = double_clip(to_double(data), 0.0, 100.0);
    //Stomper acts strangely for square waves
    //turns 0 to 100 into the duty cycle
    //with 0-->0, 1-->0.5, 100-->1
    p->duty = pow(p->exp * 0.01, LOG2OVERLOG100);
}

static void noisefactor_cb(gen_ptr g, void *data)
{
    stomperosc_data_ptr p;
    p = (stomperosc_data_ptr) g->data;
    p->noisefactor = to_double(data);
}

static void noisetype_cb(gen_ptr g, void *data)
{
    stomperosc_data_ptr p;
    p = (stomperosc_data_ptr) g->data;
    p->noisetype = to_double(data);
}

static void waveform_cb(gen_ptr g, void *data)
{
    int n = (int) to_double(data);
    stomperosc_data_ptr p;
    p = (stomperosc_data_ptr) g->data;

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
    param_double,
    waveform_cb
};

static struct param_s param_shape = {
    "shape",
    param_double,
    shape_cb
};

static struct param_s param_noisefactor = {
    "noisefactor",
    param_double,
    noisefactor_cb
};

static struct param_s param_noisetype = {
    "noisetype",
    param_double,
    noisetype_cb
};

static char *stomperosc_port_list[] = { "freq", "amp" };
static param_ptr stomperosc_param_list[]
	= { &param_waveform, &param_shape, &param_noisefactor, &param_noisetype };

struct gen_info_s stomperosc_info = {
    "stomperosc",
    "Stomper Oscillator",
    stomperosc_init,
    stomperosc_clear,
    stomperosc_note_on,
    stomperosc_note_free,
    stomperosc_tick,
    2,
    stomperosc_port_list,
    4,
    stomperosc_param_list,
};
