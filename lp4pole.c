//based on blop's lp4pole filter
//minimum clip at 20Hz to avoid some stability issues
#include <math.h>
#include <stdlib.h>

#include "gen.h"

struct lp4pole_data_s {
    float in1, in2, in3, in4;
    float out1, out2, out3, out4;
    double fmax;
};
typedef struct lp4pole_data_s lp4pole_data_t[1];
typedef struct lp4pole_data_s *lp4pole_data_ptr;

void lp4pole_init(gen_ptr g)
{
}

void lp4pole_clear(gen_ptr g)
{
}

void *lp4pole_note_on()
{
    lp4pole_data_ptr p = malloc(sizeof(lp4pole_data_t));
    p->in1 = 0.0;
    p->in2 = 0.0;
    p->in3 = 0.0;
    p->in4 = 0.0;
    p->out1 = 0.0;
    p->out2 = 0.0;
    p->out3 = 0.0;
    p->out4 = 0.0;
    p->fmax = 0.0;
    return p;
}

void lp4pole_note_free(void *data)
{
    free(data);
}

double lp4pole_tick(gen_t g, gen_data_ptr gd, double *value)
{
    double in;
    double f, tuning;
    double fsqd, coeff, fb;

    lp4pole_data_ptr p = (lp4pole_data_ptr) gd->data;
    
    if (p->fmax < value[0] * 16.0) p->fmax = value[0] * 16.0;

    f = value[1] * inv_nyquist;
    tuning = double_clip(3.13 - (f * 4.24703592), 1.56503274, 3.13);
    f = double_clip(f * tuning, 20.0 * inv_nyquist, 1.16);
    fsqd = f * f;
    coeff = fsqd * fsqd * 0.35013;
    fb = double_clip(value[2], -1.3, 4.0) * (1.0 - 0.15 * fsqd);

    f = 1.0 - f;
    in = value[0] - p->out4 * fb;
    in *= coeff;
    p->out1 = in      + 0.3 * p->in1 + f * p->out1;
    p->in1  = in;
    p->out2 = p->out1 + 0.3 * p->in2 + f * p->out2;
    p->in2  = p->out1;
    p->out3 = p->out2 + 0.3 * p->in3 + f * p->out3;
    p->in3  = p->out2;
    p->out4 = p->out3 + 0.3 * p->in4 + f * p->out4;
    p->in4  = p->out3;
    p->out4 = double_clip(p->out4, -p->fmax, p->fmax);
    p->fmax *= 0.999;

    return p->out4;
}

char *lp4pole_port_list[] = { "input", "cutoff", "resonance" };

struct gen_info_s lp4pole_info = {
    "blop4plpf",
    "blop 4-Pole LPF",
    lp4pole_init,
    lp4pole_clear,
    lp4pole_note_on,
    lp4pole_note_free,
    lp4pole_tick,
    3,
    lp4pole_port_list,
    0,
    NULL,
};
