#include <stdlib.h>
#include <math.h>

#include "gen.h"

struct adsr_data_s {
    double attack;
    double decay;
    double sustain;
    double release;
    int attacktick;
    int dtick;
    int rtick;
};

typedef struct adsr_data_s adsr_data_t[1];
typedef struct adsr_data_s *adsr_data_ptr;

struct adsr_note_data_s {
    double level;
    int age, release_age;
};

typedef struct adsr_note_data_s adsr_note_data_t[1];
typedef struct adsr_note_data_s *adsr_note_data_ptr;

void *adsr_note_on()
{
    adsr_note_data_ptr p;
    p = (adsr_note_data_ptr) malloc(sizeof(adsr_note_data_t));

    p->age = 0;
    p->release_age = 0;

    return (void *) p;
}

void adsr_note_free(void *data)
{
    free(data);
}

double adsr_tick(gen_t g, gen_data_ptr gen_data, double *value)
{
    adsr_data_ptr gd = (adsr_data_ptr) g->data;
    adsr_note_data_ptr p = (adsr_note_data_ptr) gen_data->data;

    if (gen_data->note->is_off) {
	p->release_age++;
	if (p->release_age >= gd->rtick) {
	    gen_data->alive = 0;
	    return 0.0;
	}
	return value[0] * p->level * ((double) (gd->rtick - p->release_age)) / (double) gd->rtick;
    }

    gen_data->alive = 1;
    if (p->age < gd->attacktick) {
	p->level = p->age / (double) gd->attacktick;
	p->age++;
    } else if (p->age - gd->attacktick < gd->dtick) {
	p->level = 1 - (1 - gd->sustain)
		* (p->age - gd->attacktick) / (double) gd->dtick;
	p->age++;
    } else {
	p->level = gd->sustain;
    }
    return p->level * value[0];
}

void adsr_init(gen_ptr g)
{
    g->data = malloc(sizeof(adsr_data_t));
}

void adsr_clear(gen_ptr g)
{
    free(g->data);
}

void attack_cb(gen_ptr g, double val)
{
    adsr_data_ptr p = (adsr_data_ptr) g->data;
    p->attack = val;
    p->attacktick = val * samprate;
}

void decay_cb(gen_ptr g, double val)
{
    adsr_data_ptr p = (adsr_data_ptr) g->data;
    p->decay = val;
    p->dtick = val * samprate;
}

void sustain_cb(gen_ptr g, double val)
{
    adsr_data_ptr p = (adsr_data_ptr) g->data;
    p->sustain = val;
}

void release_cb(gen_ptr g, double val)
{
    adsr_data_ptr p = (adsr_data_ptr) g->data;
    p->rtick = val * samprate;
}

struct param_s param_a = {
    "attack",
    0.1,
    attack_cb
};

struct param_s param_d = {
    "decay",
    0.5,
    decay_cb
};

struct param_s param_s = {
    "sustain",
    0.6,
    sustain_cb
};

struct param_s param_r = {
    "release",
    0.1,
    release_cb
};

char *adsr_port_list[] = { "input" };
param_ptr adsr_param_list[] = { &param_a, &param_d, &param_s, &param_r };

struct gen_info_s adsr_info = {
    "adsr",
    "ADSR Envelope",
    adsr_init,
    adsr_clear,
    adsr_note_on,
    adsr_note_free,
    adsr_tick,
    1,
    adsr_port_list,
    4,
    adsr_param_list,
};
