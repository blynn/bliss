//simple noise generator
#include <stdlib.h>
#include <math.h>

#include "gen.h"

static inline double randminus1to1()
{
    return rand() * (2.0 / (double) RAND_MAX) - 1.0;
}

typedef struct noise_data_s noise_data_t[1];
typedef struct noise_data_s *noise_data_ptr;

void *noise_note_on()
{
    return NULL;
}

void noise_note_free(void *data)
{
}

double noise_tick(gen_t g, gen_data_ptr gd, double *value)
{
    return randminus1to1();
}

void noise_init(gen_ptr g)
{
}

void noise_clear(gen_ptr g)
{
}

struct gen_info_s noise_info = {
    "noise",
    "Simple Noise",
    noise_init,
    noise_clear,
    noise_note_on,
    noise_note_free,
    noise_tick,
    0,
    NULL,
    0,
    NULL
};
