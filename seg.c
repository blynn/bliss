#include <stdlib.h>

#include "gen.h"

struct seg_data_s {
    int t0, t1;
    double deltat, deltay;
    double y0, y1;
};

typedef struct seg_data_s seg_data_t[1];
typedef struct seg_data_s *seg_data_ptr;

struct seg_note_data_s {
    int age;
};

typedef struct seg_note_data_s seg_note_data_t[1];
typedef struct seg_note_data_s *seg_note_data_ptr;

static void *seg_note_on()
{
    seg_note_data_ptr p;
    p = (seg_note_data_ptr) malloc(sizeof(seg_note_data_t));

    p->age = 0;

    return (void *) p;
}

static void seg_note_free(void *data)
{
    free(data);
}

static double seg_tick(gen_t g, gen_data_ptr gd, double *value)
{
    seg_data_ptr sdp = (seg_data_ptr) g->data;
    seg_note_data_ptr p = (seg_note_data_ptr) gd->data;
    double res;

    if (p->age > sdp->t1) {
	gd->alive = 0;
	return 0.0;
    }
    gd->alive = 1;
    if (p->age < sdp->t0) {
	p->age++;
	return 0.0;
    }
    res = sdp->y0 + (p->age - sdp->t0) * sdp->deltay / sdp->deltat;
    p->age++;
    return res * value[0];
}

static void seg_init(gen_ptr g)
{
    g->data = malloc(sizeof(seg_data_t));
}

static void seg_clear(gen_ptr g)
{
    free(g->data);
}

static void t0_cb(gen_ptr g, double val)
{
    seg_data_ptr p = (seg_data_ptr) g->data;
    p->t0 = val * samprate;
    p->deltat = p->t1 - p->t0;
}

static void t1_cb(gen_ptr g, double val)
{
    seg_data_ptr p = (seg_data_ptr) g->data;
    p->t1 = val * samprate;
    p->deltat = p->t1 - p->t0;
}

static void y0_cb(gen_ptr g, double val)
{
    seg_data_ptr p = (seg_data_ptr) g->data;
    p->y0 = val;
    p->deltay = p->y1 - p->y0;
}

static void y1_cb(gen_ptr g, double val)
{
    seg_data_ptr p = (seg_data_ptr) g->data;
    p->y1 = val;
    p->deltay = p->y1 - p->y0;
}

static struct param_s param_t0 = {
    "t0",
    0.0,
    t0_cb
};

static struct param_s param_y0 = {
    "y0",
    1.0,
    y0_cb
};

static struct param_s param_t1 = {
    "t1",
    1.0,
    t1_cb
};

static struct param_s param_y1 = {
    "y1",
    0.0,
    y1_cb
};

static char *seg_port_list[] = { "input" };
static param_ptr seg_param_list[]
	= { &param_t0, &param_y0, &param_t1, &param_y1 };

struct gen_info_s seg_info = {
    "seg",
    "Line Segment",
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
