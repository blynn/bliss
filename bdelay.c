#include <stdlib.h>
#include <math.h>
#include "buzz_machine.h"
#include "util.h"

enum {
    max_delay = 100000,
    max_taps = 8,
    unit_tick = 0,
    unit_ms,
    unit_sample,
    unit_256
};

struct tval_s {
    double *lbuffer, *rbuffer;
    int bufsize;
    int unit;
    int length;
    int pos;
    double feedback;
    double wetout;
};

struct gval_s {
    int drythru;
};

struct bdelay_data_s {
    struct gval_s gval;
    struct tval_s tval[max_taps];
};

typedef struct bdelay_data_s *bdelay_data_ptr;

static bdelay_data_ptr data_at(machine_ptr m)
{
    return (bdelay_data_ptr) m->data;
}

static double silent_enough;

static int first = 1;

static void compute_bufsize(machine_ptr m, struct tval_s *t)
{
    switch (t->unit) {
	case unit_ms:
	    t->bufsize = samprate * t->length / 1000;
	    if (t->bufsize < 1) t->bufsize = 1;
	    break;
	case unit_sample:
	    t->bufsize = t->length;
	    break;
	case unit_tick:
	    t->bufsize = m->song->samptick * t->length;
	    break;
	case unit_256:
	    t->bufsize = m->song->samptick * t->length >> 8;
	    if (t->bufsize < 1) t->bufsize = 1;
	    break;
    }
    if (t->bufsize > max_delay) t->bufsize = max_delay;
    if (t->pos >= t->bufsize) t->pos = 0;
}

static void handle_length(machine_ptr m, int track, int val)
{
    bdelay_data_ptr p = data_at(m);
    p->tval[track].length = val;
    compute_bufsize(m, &p->tval[track]);
}

static void handle_unit(machine_ptr m, int track, int val)
{
    bdelay_data_ptr p = data_at(m);
    p->tval[track].unit = val;
    compute_bufsize(m, &p->tval[track]);
}

static void handle_drythru(machine_ptr m, int track, int val)
{
    bdelay_data_ptr p = data_at(m);
    p->gval.drythru = val;
}

static void handle_feedback(machine_ptr m, int track, int val)
{
    bdelay_data_ptr p = data_at(m);
    p->tval[track].feedback = (val - 64) * (1.0 / 64.0);
}

static void handle_wetout(machine_ptr m, int track, int val)
{
    bdelay_data_ptr p = data_at(m);
    p->tval[track].wetout = val * (1.0 / 128.0);
}

static struct buzz_param_s param_length = {
    pt_word,
    "Length",
    "Length in length units",
    1,
    32768,
    65535,
    MPF_STATE,
    3,
    handle_length,
};

static struct buzz_param_s param_drythru = {
    pt_byte,
    "Dry thru",
    "Dry thru (1 = yes, 0 = no)",
    -1,
    -1,
    SWITCH_NO,
    MPF_STATE,
    SWITCH_ON,
    handle_drythru,
};

static struct buzz_param_s param_unit = {
    pt_byte,
    "Length unit",
    "Length unit (0 = tick (default), 1 = ms, 2 = sample, 3 = 256th of tick",
    0,
    3,
    0xFF,
    MPF_STATE,
    0,
    handle_unit,
};

static struct buzz_param_s param_feedback = {
    pt_byte,
    "Feedback",
    "Feedback (00 = -100%, 40 = 0%, 80 = 100%)",
    0,
    0x80,
    0xFF,
    MPF_STATE,
    0x60,
    handle_feedback,
};

static struct buzz_param_s param_wetout = {
    pt_byte,
    "Wet out",
    "Wet out (00 = 0%, 80 = 100%)",
    0,
    0x80,
    0xFF,
    MPF_STATE,
    0x10,
    handle_wetout,
};

void buzz_param_init()
{
    global_param(&param_drythru);
    track_param(&param_length);
    track_param(&param_unit);
    track_param(&param_feedback);
    track_param(&param_wetout);
}

static void bdelay_init(machine_t m)
{
    int i;
    bdelay_data_ptr p;
    m->data = malloc(sizeof(struct bdelay_data_s));
    p = (bdelay_data_ptr) m->data;

    if (first) {
	first = 0;
	silent_enough = 1.0 / log(2);
    }
    p->gval.drythru = 1;
    for (i=0; i<max_taps; i++) {
	p->tval[i].lbuffer = (double *) calloc(sizeof(double), max_delay);
	p->tval[i].rbuffer = (double *) calloc(sizeof(double), max_delay);
	p->tval[i].unit = unit_tick;
	p->tval[i].length = 3;
	p->tval[i].pos = 0;
	p->tval[i].feedback = 0.3f;
	p->tval[i].wetout = 0;
	compute_bufsize(m, &p->tval[i]);
    }
    p->tval[0].wetout = 0.3f;
}

static void bdelay_clear(machine_t m)
{
    int i;
    bdelay_data_ptr p = (bdelay_data_ptr) m->data;
    for (i=0; i<max_taps; i++) {
	free(p->tval[i].lbuffer);
	free(p->tval[i].rbuffer);
    }
    free(m->data);
}

static void bdelay_tick(machine_t m)
{
}

static void bdelay_work(machine_t m, double *l, double *r)
{
    int i;
    bdelay_data_ptr p = data_at(m);

    for (i=0; i<max_taps; i++) {
	struct tval_s* t = &p->tval[i];
	double delay;

	delay = t->lbuffer[t->pos];
	t->lbuffer[t->pos] = delay * t->feedback + *l;
	*l = p->gval.drythru * *l + delay * t->wetout;

	delay = t->rbuffer[t->pos];
	t->rbuffer[t->pos] = delay * t->feedback + *r;
	*r = p->gval.drythru * *r + delay * t->wetout;

	t->pos++;
	if (t->pos >= t->bufsize) t->pos = 0;
    }
}

void buzz_machine_info_init(buzz_machine_info_ptr mi) {
    mi->name = "Delay";
    mi->id = "Jeskola Delay";
    mi->type = machine_effect;
    mi->init = bdelay_init;
    mi->clear = bdelay_clear;
    mi->work = bdelay_work;
    mi->tick = bdelay_tick;
    mi->track_max = max_taps;
}
