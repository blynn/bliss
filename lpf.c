#include <math.h>
#include <stdlib.h>

#include "gen.h"

struct lpf_data_s {
    double a0, a1, a2, b1, b2;
};
typedef struct lpf_data_s *lpf_data_ptr;
typedef struct lpf_data_s lpf_data_t[1];

struct lpf_history_s {
    double x1, x2, y1, y2;
};
typedef struct lpf_history_s lpf_history_t[1];
typedef struct lpf_history_s *lpf_history_ptr;

void lpf_init(gen_ptr g)
{
    lpf_data_ptr p;
    g->data = malloc(sizeof(lpf_data_t));
    p = (lpf_data_ptr) g->data;
}

void compute_taps(lpf_data_ptr p, double cutoff)
{
    double b2, b1, bd;

    b2 = 1.0 / tan(M_PI * cutoff);
    b1 = b2;
    b2 = b2 * b2;
    bd = 1.0 / (b1 + b2 + 1.0);

    p->b1 = -(2.0 - 2.0 * b2) * bd;
    p->b2 = -(b2 - b1 + 1.0) * bd;
    p->a0 = bd;
    p->a1 = 2 * bd;
    p->a2 = bd;
}

void lpf_clear(gen_ptr g)
{
    free(g->data);
}

double lowpass(gen_ptr g, struct lpf_history_s *h, double x)
{
    double y;
    lpf_data_ptr p;

    p = (lpf_data_ptr) g->data;

    y = x * p->a0 + h->x1 * p->a1 + h->x2 * p->a2
	+ h->y1 * p->b1 + h->y2 * p->b2;
    h->x2 = h->x1;
    h->x1 = x;
    h->y2 = h->y1;
    h->y1 = y;
    return y;
}

void *lpf_note_on()
{
    lpf_history_ptr p = malloc(sizeof(lpf_history_t));
    p->x1 = 0.0;
    p->x2 = 0.0;
    p->y1 = 0.0;
    p->y2 = 0.0;
    return p;
}

void lpf_note_free(void *data)
{
    free(data);
}

double lpf_tick(gen_t g, void *data, double *value)
{
    double res;
    res = lowpass(g, (struct lpf_history_s *) data, value[0]);
    return res;
}

void cutoff_cb(gen_ptr g, double val)
{
    compute_taps((lpf_data_ptr) g->data, val);
}

char *lpf_port_list[] = { "input" };

struct param_s param_cutoff = {
    "cutoff",
    0.5,
    cutoff_cb
};

param_ptr lpf_param_list[] = { &param_cutoff };

struct gen_info_s lpf_info = {
    "lpf",
    lpf_init,
    lpf_clear,
    lpf_note_on,
    lpf_note_free,
    lpf_tick,
    1,
    lpf_port_list,
    1,
    lpf_param_list,
};
