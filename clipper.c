#include <stdlib.h>
#include <math.h>

#include "gen.h"

struct clipper_data_s {
    double min, max;
};

typedef struct clipper_data_s clipper_data_t[1];
typedef struct clipper_data_s *clipper_data_ptr;

void *clipper_note_on()
{
    return NULL;
}

void clipper_note_free(void *data)
{
}

double clipper_tick(gen_t g, gen_data_ptr gen_data, double *value)
{
    clipper_data_ptr gd = (clipper_data_ptr) g->data;

    return double_clip(value[0], gd->min, gd->max);
}

void clipper_init(gen_ptr g)
{
    g->data = malloc(sizeof(clipper_data_t));
}

void clipper_clear(gen_ptr g)
{
    free(g->data);
}

void min_cb(gen_ptr g, double val)
{
    clipper_data_ptr p = (clipper_data_ptr) g->data;
    p->min = val;
}

void max_cb(gen_ptr g, double val)
{
    clipper_data_ptr p = (clipper_data_ptr) g->data;
    p->max = val;
}

struct param_s param_min = {
    "min",
    -1.0,
    min_cb
};

struct param_s param_max = {
    "max",
    1.0,
    max_cb
};

char *clipper_port_list[] = { "input" };
param_ptr clipper_param_list[] = { &param_min, &param_max };

struct gen_info_s clipper_info = {
    "clipper",
    "Clipper",
    clipper_init,
    clipper_clear,
    clipper_note_on,
    clipper_note_free,
    clipper_tick,
    1,
    clipper_port_list,
    2,
    clipper_param_list,
};
