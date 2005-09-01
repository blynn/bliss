//Stomper Hyperion style envelopes
//a bit expensive: uses pow() every tick
//Stomper interpolates between (t0, y0) and (t1, y1)
//by scaling the curve x^s appropriately,
//where s = "curveshape"
//note this mesaures time in seconds, not milliseconds
#include <stdlib.h>
#include <math.h>

#include "gen.h"

struct stomperenv_data_s {
    int t0, t1;
    double deltat, deltay;
    double y0, y1;
    double exp;
};

typedef struct stomperenv_data_s stomperenv_data_t[1];
typedef struct stomperenv_data_s *stomperenv_data_ptr;

struct stomperenv_note_data_s {
    int age;
};

typedef struct stomperenv_note_data_s stomperenv_note_data_t[1];
typedef struct stomperenv_note_data_s *stomperenv_note_data_ptr;

static void *stomperenv_note_on()
{
    stomperenv_note_data_ptr p;
    p = (stomperenv_note_data_ptr) malloc(sizeof(stomperenv_note_data_t));

    p->age = 0;

    return (void *) p;
}

static void stomperenv_note_free(void *data)
{
    free(data);
}

static double stomperenv_tick(gen_t g, gen_data_ptr gd, double *value)
{
    stomperenv_data_ptr sdp = (stomperenv_data_ptr) g->data;
    stomperenv_note_data_ptr p = (stomperenv_note_data_ptr) gd->data;
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
    p->age++;
    res = sdp->y0 + sdp->deltay * pow((p->age - sdp->t0) / sdp->deltat, sdp->exp);
    return res;
}

static void stomperenv_init(gen_ptr g)
{
    g->data = malloc(sizeof(stomperenv_data_t));
    assign_double(g, 1, 1.0);
    assign_double(g, 2, 1.0);
    assign_double(g, 4, 1.0);
}

static void stomperenv_clear(gen_ptr g)
{
    free(g->data);
}

static void t0_cb(gen_ptr g, void *data)
{
    stomperenv_data_ptr p = (stomperenv_data_ptr) g->data;
    p->t0 = to_double(data) * samprate;
    p->deltat = p->t1 - p->t0;
}

static void t1_cb(gen_ptr g, void *data)
{
    stomperenv_data_ptr p = (stomperenv_data_ptr) g->data;
    p->t1 = to_double(data) * samprate;
    p->deltat = p->t1 - p->t0;
}

static void y0_cb(gen_ptr g, void *data)
{
    stomperenv_data_ptr p = (stomperenv_data_ptr) g->data;
    p->y0 = to_double(data);
    p->deltay = p->y1 - p->y0;
}

static void y1_cb(gen_ptr g, void *data)
{
    stomperenv_data_ptr p = (stomperenv_data_ptr) g->data;
    p->y1 = to_double(data);
    p->deltay = p->y1 - p->y0;
}

static void shape_cb(gen_ptr g, void *data)
{
    stomperenv_data_ptr p = (stomperenv_data_ptr) g->data;
    p->exp = to_double(data);
}

static struct param_s param_t0 = {
    "t0",
    param_double,
    t0_cb
};

static struct param_s param_y0 = {
    "y0",
    param_double,
    y0_cb
};

static struct param_s param_t1 = {
    "t1",
    param_double,
    t1_cb
};

static struct param_s param_y1 = {
    "y1",
    param_double,
    y1_cb
};

static struct param_s param_shape = {
    "curveshape",
    param_double,
    shape_cb
};

static param_ptr stomperenv_param_list[]
	= { &param_t0, &param_y0, &param_t1, &param_y1, &param_shape };

struct gen_info_s stomperenv_info = {
    "stomperenv",
    "Stomper Envelope",
    stomperenv_init,
    stomperenv_clear,
    stomperenv_note_on,
    stomperenv_note_free,
    stomperenv_tick,
    0,
    NULL,
    5,
    stomperenv_param_list,
};
