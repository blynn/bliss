//One zero filter, with normalized peak gain
//barely noticeable when used as lowpass
//
//H(z) = 1 - r z^-1
//
//peak gain = 1+r for r > 0
//          = 1-r for r < 0
#include <math.h>
#include <stdlib.h>

#include "gen.h"

struct onezero_history_s {
    double x1;
};
typedef struct onezero_history_s onezero_history_t[1];
typedef struct onezero_history_s *onezero_history_ptr;

void onezero_init(gen_ptr g)
{
}

void onezero_clear(gen_ptr g)
{
}

void *onezero_note_on()
{
    onezero_history_ptr p = malloc(sizeof(onezero_history_t));
    p->x1 = 0.0;
    return p;
}

void onezero_note_free(void *data)
{
    free(data);
}

double onezero_tick(gen_t g, gen_data_ptr gd, double *value)
{
    onezero_history_ptr h = (onezero_history_ptr) gd->data;
    double y;
    //double r = double_clip(value[1], -1.0, 1.0);
    double r = value[1];
    double a;
    double x = value[0];

    if (r > 0.0) {
	a = 1.0 + r;
    } else {
	a = 1.0 - r;
    }
    y = (x - r * h->x1) / a;
    h->x1 = x;
    return y;
}

char *onezero_port_list[] = { "input", "zero" };

struct gen_info_s onezero_info = {
    "onezero",
    "One Zero Filter",
    onezero_init,
    onezero_clear,
    onezero_note_on,
    onezero_note_free,
    onezero_tick,
    2,
    onezero_port_list,
    0,
    NULL,
};
