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

struct bdistortion_data_s {
    double threshold;
    double clamp;
    double negthreshold;
    double negclamp;
    double amount;
};

typedef struct bdistortion_data_s *bdistortion_data_ptr;

static bdistortion_data_ptr data_at(machine_ptr m)
{
    return (bdistortion_data_ptr) m->data;
}

static void handle_threshold(machine_ptr m, int track, int val)
{
    bdistortion_data_ptr p = data_at(m);
    p->threshold = val * 16.0;
}

static void handle_clamp(machine_ptr m, int track, int val)
{
    bdistortion_data_ptr p = data_at(m);
    p->clamp = val * 16.0;
}

static void handle_negthreshold(machine_ptr m, int track, int val)
{
    bdistortion_data_ptr p = data_at(m);
    p->negthreshold = val * -16.0;
}

static void handle_negclamp(machine_ptr m, int track, int val)
{
    bdistortion_data_ptr p = data_at(m);
    p->negclamp = val * -16.0;
}

static void handle_amount(machine_ptr m, int track, int val)
{
    bdistortion_data_ptr p = data_at(m);
    p->amount = val * (1.0 / 0x7f);
}

static struct buzz_param_s param_threshold = {
    pt_word,
    "+threshold",
    "Positive threshold level",
    0,
    0xfffe,
    0xffff,
    MPF_STATE,
    0x200,
    handle_threshold,
};

static struct buzz_param_s param_clamp = {
    pt_word,
    "+clamp",
    "Positive clamp level",
    0,
    0xfffe,
    0xffff,
    MPF_STATE,
    0x1000,
    handle_clamp,
};

static struct buzz_param_s param_negthreshold = {
    pt_word,
    "-threshold",
    "Negative threshold level",
    0,
    0xfffe,
    0xffff,
    MPF_STATE,
    0x200,
    handle_negthreshold,
};

static struct buzz_param_s param_negclamp = {
    pt_word,
    "-clamp",
    "Negative clamp level",
    0,
    0xfffe,
    0xffff,
    MPF_STATE,
    0x1000,
    handle_negclamp,
};

static struct buzz_param_s param_amount = {
    pt_byte,
    "Amount",
    "Amount",
    0,
    0x7f,
    0xFF,
    MPF_STATE,
    0x7f,
    handle_amount,
};

void buzz_param_init()
{
    global_param(&param_threshold);
    global_param(&param_clamp);
    global_param(&param_negthreshold);
    global_param(&param_negclamp);
    global_param(&param_amount);
}

static void bdistortion_init(machine_t m)
{

    bdistortion_data_ptr p;
    m->data = malloc(sizeof(struct bdistortion_data_s));
    p = data_at(m);

    p->threshold = 65536 * 16.0;
    p->clamp = 65536 * 16.0;
    p->negthreshold = -65536 * 16.0;
    p->negclamp = -65536 * 16.0;
    p->amount = 1.0;
}

static void bdistortion_clear(machine_t m)
{
    free(m->data);
}

static void bdistortion_tick(machine_t m)
{
}

static void bdistortion_work(machine_t m, double *l, double *r)
{
    bdistortion_data_ptr p = data_at(m);

    double drymix = 1.0 - p->amount;
    double clamp = p->amount * p->clamp;
    double threshold = p->threshold;
    double negclamp = p->amount * p->negclamp;
    double negthreshold = p->negthreshold;

    if (*l >= threshold) {
	*l = (*l * drymix + clamp);
    } else if (*l <= negthreshold) {
	*l = (*l * drymix + negclamp);
    }
    if (*r >= threshold) {
	*r = (*r * drymix + clamp);
    } else if (*r <= negthreshold) {
	*r = (*r * drymix + negclamp);
    }
}

void buzz_machine_info_init(buzz_machine_info_ptr mi) {
    mi->name = "Dist";
    mi->id = "Jeskola Distortion";
    mi->type = machine_effect;
    mi->init = bdistortion_init;
    mi->clear = bdistortion_clear;
    mi->work = bdistortion_work;
    mi->tick = bdistortion_tick;
    mi->track_max = 0;
}
