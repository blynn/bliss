#include <stdlib.h>
#include <math.h>
#include "buzz_machine.h"
#include "util.h"

enum {
    track_max = 16,
    resolution = 1024,
    type_saw = 0,
    type_sine,
};

struct bbass2_data_s {
    int ttl;
    int type;
    double f;
    double x;

    double x1, x2, y1, y2;
    double a0, a1, a2, b1, b2;

    double cutoff;
    double recipq;
};
typedef struct bbass2_data_s *bbass2_data_ptr;

static bbass2_data_ptr data_at(machine_ptr m, int track)
{
    return &((bbass2_data_ptr) m->data)[track];
}

static int first = 1;
static double sintable[resolution];

static double lowpass(bbass2_data_ptr p, double x)
{
    double y;

    y = x * p->a0 + p->x1 * p->a1 + p->x2 * p->a2
	+ p->y1 * p->b1 + p->y2 * p->b2;
    p->x2 = p->x1;
    p->x1 = x;
    p->y2 = p->y1;
    p->y1 = y;
    return y;
}

static void computetaps(bbass2_data_ptr p)
{
    double b2, b1, bd;

    b2 = 1.0 / tan(M_PI * p->cutoff);
    b1 = p->recipq * b2;
    b2 = b2 * b2;
    bd = 1.0 / (b1 + b2 + 1.0);

    p->b1 = -(2.0 - 2.0 * b2) * bd;
    p->b2 = -(b2 - b1 + 1.0) * bd;
    p->a0 = bd;
    p->a1 = 2 * bd;
    p->a2 = bd;
}

static void handle_cutoff(machine_ptr m, int track, int val)
{
    bbass2_data_ptr p = data_at(m, track);
    p->cutoff = ((double) (val + 1)) / 256;
    computetaps(p);
}

static void handle_resonance(machine_ptr m, int track, int val)
{
    bbass2_data_ptr p = data_at(m, track);
    p->recipq = 1.0 - ((double) val) / 256;
    computetaps(p);
}

static void handle_note(machine_ptr m, int track, int val)
{
    bbass2_data_ptr p = data_at(m, track);
    p->f = note_to_freq(val);
    p->x = 0;
    p->ttl = m->song->samptick;
}

static void handle_length(machine_ptr m, int track, int val)
{
    bbass2_data_ptr p = data_at(m, track);
    p->ttl = val * m->song->samptick;
}

static void handle_unknown(machine_ptr m, int track, int val)
{
}

static struct buzz_param_s param_cutoff = {
    pt_byte,
    "Cutoff",
    "Cutoff",
    1,
    0x80,
    0xFF,
    MPF_STATE,
    0,
    handle_cutoff,
};

static struct buzz_param_s param_resonance = {
    pt_byte,
    "Resonance",
    "Resonance",
    1,
    0x80,
    0xFF,
    MPF_STATE,
    0,
    handle_resonance,
};

static struct buzz_param_s param_cem = {
    pt_byte,
    "CEM",
    "Cutoff Envelope Modulation",
    1,
    0x80,
    0xFF,
    MPF_STATE,
    0,
    handle_unknown,
};

static struct buzz_param_s param_ced = {
    pt_byte,
    "CED",
    "Cutoff Envelope Decay",
    1,
    0x80,
    0xFF,
    MPF_STATE,
    0,
    handle_unknown,
};

static struct buzz_param_s param_waveform = {
    pt_byte,
    "Waveform",
    "Waveform",
    0,
    4,
    0xFF,
    MPF_STATE,
    1,
    handle_unknown,
};

static struct buzz_param_s param_note = {
    pt_note,
    "Note",
    "Note",
    NOTE_MIN,
    NOTE_MAX,
    NOTE_NO,
    0,
    NOTE_NO,
    handle_note,
};

static struct buzz_param_s param_slide = {
    pt_note,
    "Slide",
    "Slide End Note",
    NOTE_MIN,
    NOTE_MAX,
    NOTE_NO,
    0,
    NOTE_NO,
    handle_unknown,
};

static struct buzz_param_s param_vol = {
    pt_byte,
    "Vol",
    "Volume",
    1,
    0xFE,
    0xFF,
    MPF_STATE,
    0,
    handle_unknown,
};

static struct buzz_param_s param_len = {
    pt_byte,
    "Len",
    "Length of note in ticks",
    1,
    0xFF,
    0,
    MPF_STATE,
    0,
    handle_length,
};

void buzz_param_init()
{
    track_param(&param_cutoff);
    track_param(&param_resonance);
    track_param(&param_cem);
    track_param(&param_ced);
    track_param(&param_waveform);
    track_param(&param_note);
    track_param(&param_vol);
    track_param(&param_len);
    track_param(&param_slide);
}

static void bbass2_static_init()
{
    int i;
    for (i=0; i<resolution; i++) {
	sintable[i] = 0.5 * sin(((double) i / (double) resolution) * M_PI * 2);
    }
}

static void bbass2_init(machine_t m)
{
    int i;
    m->data = malloc(track_max * sizeof(struct bbass2_data_s));
    if (first) {
	first = 0;
	bbass2_static_init();
    }
    for (i=0; i<track_max; i++) {
	bbass2_data_ptr p = data_at(m, i);
	p->ttl = 0;
	p->type = type_saw;
	p->x1 = p->x2 = 0;
	p->y1 = p->y2 = 0;
	handle_cutoff(m, i, 16);
	handle_resonance(m, i, 16);
	computetaps(p);
    }
}

static void bbass2_clear(machine_t m)
{
    free(m->data);
}

static void bbass2_tick(machine_t m)
{
}

static void bbass2_work(machine_t m, double *l, double *r)
{
    double x;
    double d;
    int i, j;

    for (i=0; i<track_max; i++) {
	bbass2_data_ptr p = &((bbass2_data_ptr) m->data)[i];
	if (p->ttl) {
	    x = p->x;
	    switch (p->type) {
		case type_sine:
		    j = resolution * x / samprate;
		    d = sintable[j];
		    break;
		case type_saw:
		    d = 0.5 - x / samprate;
		    break;
		default:
		    d = 0;
		    break;
	    }
	    x += p->f;
	    if (x >= samprate) x -= samprate;
	    p->x = x;
	    //very simple decay
	    if (p->ttl < 5000) {
		d *= (double) p->ttl / 5000;
	    }
	    p->ttl--;
	    d = lowpass(p, d);
	    d *= 32768 / 4;
	    *l += d;
	    *r += d;
	}
    }
}

void buzz_machine_info_init(buzz_machine_info_ptr mi) {
    mi->name = "Bass2";
    mi->id = "Jeskola Bass 2";
    mi->type = machine_generator;
    mi->init = bbass2_init;
    mi->clear = bbass2_clear;
    mi->work = bbass2_work;
    mi->tick = bbass2_tick;
    mi->track_max = track_max;
}
