#include <stdlib.h>
#include <math.h>
#include "buzz_machine.h"
#include "util.h"

enum {
    max_tracks = 8,
    egs_attack = 0,
    egs_sustain,
    egs_release,
    egs_none,
};

#define MIN_AMP (0.0001 * (32768.0 / 0x7fffffff))

//double const oolog2 = 1.0 / log(2);

static inline double calcstep(double from, double to, int time)
{
    /*
    assert(from > 0);
    assert(to > 0);
    assert(time > 0);
    */
    return pow(to / from, 1.0 / time);
}

struct tval_s {
    int attack;
    int sustain;
    int release;
    double volume;

    int egstage;
    int egcount;
    double amp;
    double ampstep;

    int pos;
    int step;
    int rand_state;
    double s1, s2;
};

struct bnoise_data_s {
    struct tval_s tval[max_tracks];
};
typedef struct bnoise_data_s *bnoise_data_ptr;

inline static bnoise_data_ptr data_at(machine_ptr m)
{
    return (bnoise_data_ptr) m->data;
}

inline static int ms2samples(int val)
{
    return samprate * val / 1000;
}

static void handle_attack(machine_ptr m, int track, int val)
{
    bnoise_data_ptr p = data_at(m);
    p->tval[track].attack = ms2samples(val);
}

static void handle_release(machine_ptr m, int track, int val)
{
    bnoise_data_ptr p = data_at(m);
    p->tval[track].release = ms2samples(val);
}

static void handle_sustain(machine_ptr m, int track, int val)
{
    bnoise_data_ptr p = data_at(m);
    p->tval[track].sustain = ms2samples(val);
}

static void handle_color(machine_ptr m, int track, int val)
{
    bnoise_data_ptr p = data_at(m);
    p->tval[track].step = val * 16;
}

static void handle_volume(machine_ptr m, int track, int val)
{
    bnoise_data_ptr p = data_at(m);
    p->tval[track].volume = val * (1.0 / 0x80);
}

static void handle_trigger(machine_ptr m, int track, int val)
{
    bnoise_data_ptr p = data_at(m);
    p->tval[track].egstage = egs_attack;
    p->tval[track].egcount = p->tval[track].attack;
    p->tval[track].amp = MIN_AMP;
    p->tval[track].ampstep = calcstep(MIN_AMP, p->tval[track].volume * (32768.0 / 0x7fffffff), p->tval[track].attack);
}

static struct buzz_param_s param_attack = {
    pt_word,
    "Attack",
    "Attack time in ms",
    1,
    0xffff,
    0,
    MPF_STATE,
    16,
    handle_attack,
};

static struct buzz_param_s param_sustain = {
    pt_word,
    "Sustain",
    "Sustain time in ms",
    1,
    0xffff,
    0,
    MPF_STATE,
    16,
    handle_sustain
};

static struct buzz_param_s param_release = {
    pt_word,
    "Release",
    "Release time in ms",
    1,
    0xffff,
    0,
    MPF_STATE,
    512,
    handle_release
};
static struct buzz_param_s param_color = {
    pt_word,
    "Color",
    "Noise color (0=black, 1000=white)",
    0,
    0x1000,
    0xffff,
    MPF_STATE,
    0x1000,
    handle_color
};

static struct buzz_param_s param_volume = {
    pt_byte,
    "Volume",
    "Volume [sustain level] (0=0%, 80=100%, FE=~200%)",
    0,
    0xfe,
    0xff,
    MPF_STATE,
    0x80,
    handle_volume
};

static struct buzz_param_s param_trigger = {
    pt_switch,
    "Trigger",
    "Trigger (1=on, 0=off)",
    -1,
    -1,
    SWITCH_NO,
    0,
    0,
    handle_trigger
};

void buzz_param_init()
{
    track_param(&param_attack);
    track_param(&param_sustain);
    track_param(&param_release);
    track_param(&param_color);
    track_param(&param_volume);
    track_param(&param_trigger);
}

static void bnoise_init(machine_t m)
{
    int i;
    bnoise_data_ptr p;
    m->data = malloc(sizeof(struct bnoise_data_s));
    p = data_at(m);
    for (i=0; i<max_tracks; i++) {
	p->tval[i].egstage = egs_none;
	p->tval[i].attack = ms2samples(16);
	p->tval[i].sustain = ms2samples(16);
	p->tval[i].release = ms2samples(16);

	p->tval[i].pos = 0;
	p->tval[i].step = 65536;
	p->tval[i].volume = 1.0;
	p->tval[i].s1 = 0;
	p->tval[i].s2 = 0;
	p->tval[i].rand_state = 0x16BA2118;
    }
}

static void bnoise_clear(machine_t m)
{
    free(m->data);
}

static double noise(struct tval_s *t)
{
    double d;
    d = t->s1 + (t->s2 - t->s1) * (t->pos * 1.0 / 65536.0);
    t->amp *= t->ampstep;
    t->pos += t->step;
    if (t->pos & 65536) {
	t->s1 = t->s2;
	t->rand_state = ((t->rand_state * 1103515245 + 12345) & 0x7fffffff) - 0x40000000;
	t->s2 = t->rand_state * t->amp;
	t->pos -= 65536;
    }

    return d;
}

static double generate(struct tval_s *t)
{
    double d;
    if (t->egstage != egs_none) {
	d = noise(t);
    } else {
	d = 0;
    }
    t->egcount--;
    if (!t->egcount) {
	t->egstage++;
	switch(t->egstage) {
	    case egs_sustain:
		t->egcount = t->sustain;
		t->ampstep = 1.0;
		break;
	    case egs_release:
		t->egcount = t->release;
		t->ampstep = calcstep(t->amp, MIN_AMP, t->release);
		break;
	    case egs_none:
		t->egcount = 0x7fffffff;
		break;
	}
    }
    return d;
}

static void bnoise_tick(machine_t m)
{
}

static void bnoise_work(machine_t m, double *l, double *r)
{
    int i;
    bnoise_data_ptr p = data_at(m);

    for (i=0; i<max_tracks; i++) {
	if (p->tval[i].egstage != egs_none) {
	    double d;
	    d = generate(&p->tval[i]);
	    *l += d;
	    *r += d;
	}
    }
}

void buzz_machine_info_init(buzz_machine_info_ptr mi) {
    mi->name = "Noise";
    mi->id = "Jeskola Noise Generator";
    mi->type = machine_generator;
    mi->init = bnoise_init;
    mi->clear = bnoise_clear;
    mi->work = bnoise_work;
    mi->tick = bnoise_tick;
    mi->track_max = max_tracks;
}
