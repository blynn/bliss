#include <stdlib.h>

#include "gen.h"

struct seg_data_s {
    int t0, t1;
    double y0, y1;
};

typedef struct seg_data_s seg_data_t[1];
typedef struct seg_data_s *seg_data_ptr;

struct seg_note_data_s {
    int age;
    int dead;
};

typedef struct seg_note_data_s seg_note_data_t[1];
typedef struct seg_note_data_s *seg_note_data_ptr;

void *seg_note_on()
{
    seg_note_data_ptr p;
    p = (seg_note_data_ptr) malloc(sizeof(seg_note_data_t));

    p->age = 0;
    p->dead = 0;

    return (void *) p;
}

void seg_note_free(void *data)
{
    free(data);
}

double seg_tick(gen_t g, void *data, double *value)
{
    seg_data_ptr gd = (seg_data_ptr) g->data;
    seg_note_data_ptr p = (seg_note_data_ptr) data;
    double res;

    if (p->age > gd->t1) {
	p->dead = 1;
	return 0.0;
    }
    if (p->age < gd->t0) {
	p->age++;
	return 0.0;
    }
    res = gd->y0
	+ (p->age - gd->t0) * (gd->y1 - gd->y0) / ((double) (gd->t1 - gd->t0));
    p->age++;
    return res * value[0];
}

double seg_note_off_tick(gen_ptr g, void *data, double *value, int *dead)
{
    seg_note_data_ptr p = (seg_note_data_ptr) data;

    if (p->dead) {
	*dead = 1;
	return 0.0;
    }

    else return seg_tick(g, data, value);
}

void seg_init(gen_ptr g)
{
    g->data = malloc(sizeof(seg_data_t));
    g->note_off_tick = seg_note_off_tick;
}

void seg_clear(gen_ptr g)
{
    free(g->data);
}

void t0_cb(gen_ptr g, double val)
{
    seg_data_ptr p = (seg_data_ptr) g->data;
    p->t0 = val * 44100;
}

void t1_cb(gen_ptr g, double val)
{
    seg_data_ptr p = (seg_data_ptr) g->data;
    p->t1 = val * 44100;
}

void y0_cb(gen_ptr g, double val)
{
    seg_data_ptr p = (seg_data_ptr) g->data;
    p->y0 = val;
}

void y1_cb(gen_ptr g, double val)
{
    seg_data_ptr p = (seg_data_ptr) g->data;
    p->y1 = val;
}

struct param_s param_t0 = {
    "t0",
    0.0,
    t0_cb
};

struct param_s param_y0 = {
    "y0",
    1.0,
    y0_cb
};

struct param_s param_t1 = {
    "t1",
    1.0,
    t1_cb
};

struct param_s param_y1 = {
    "y1",
    0.0,
    y1_cb
};

char *seg_port_list[] = { "input" };
param_ptr seg_param_list[] = { &param_t0, &param_y0, &param_t1, &param_y1 };

struct gen_info_s seg_info = {
    "seg",
    seg_init,
    seg_clear,
    seg_note_on,
    seg_note_free,
    seg_tick,
    1,
    seg_port_list,
    4,
    seg_param_list,
};
