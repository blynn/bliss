//Based on blop random wave generator
#include <stdlib.h>
#include <math.h>

#include "gen.h"

static inline double randminus1to1()
{
    return rand() * (2.0 / (double) RAND_MAX) - 1.0;
}

struct random_wave_data_s {
    double phase;
    double x0, x1;
};

typedef struct random_wave_data_s random_wave_data_t[1];
typedef struct random_wave_data_s *random_wave_data_ptr;

void *random_wave_note_on()
{
    random_wave_data_ptr res = (random_wave_data_ptr) malloc(sizeof(random_wave_data_t));
    res->x0 = randminus1to1();
    res->x1 = randminus1to1();
    res->phase = 0.0;
    return (void *) res;
}

void random_wave_note_free(void *data)
{
    free(data);
}

double random_wave_tick(gen_t g, gen_data_ptr gd, double *value)
{
    double res;
    double freq = double_clip(value[0], 0.0, nyquist);
    double smooth = double_clip(value[1], 0.0, 1.0);
    random_wave_data_ptr p = (random_wave_data_ptr) gd->data;
    double interval = (1.0 - smooth) * 0.5;

    if (p->phase < interval) {
	res = 1.0;
    } else if (p->phase > 1.0 - interval) {
	res = -1.0;
    } else if (interval > 0.0) {
	res = cos((p->phase - interval) / smooth * M_PI);
    } else {
	res = cos(p->phase * M_PI);
    }
    res *= (p->x1 - p->x0) * 0.5;
    res += (p->x1 + p->x0) * 0.5;

    p->phase += freq * inv_nyquist;
    if (p->phase > 1.0) {
	p->phase -= 1.0;
	p->x0 = p->x1;
	p->x1 = randminus1to1();
    }
    return res;
}

void random_wave_init(gen_ptr g)
{
}

void random_wave_clear(gen_ptr g)
{
}

char *random_wave_port_list[] = { "freq", "smooth" };

struct gen_info_s random_wave_info = {
    "blop_random_wave",
    "Random Wave",
    random_wave_init,
    random_wave_clear,
    random_wave_note_on,
    random_wave_note_free,
    random_wave_tick,
    2,
    random_wave_port_list,
    0,
    NULL
};
