#include <stdlib.h>
#include <math.h>

#include "gen.h"

void *dummy_note_on()
{
    return NULL;
}

void dummy_note_free(void *data)
{
}

double dummy_tick(gen_t g, gen_data_ptr gd, double *value)
{
    return 0;
}

void dummy_init(gen_ptr g)
{
}

void dummy_clear(gen_ptr g)
{
}

struct gen_info_s dummy_info = {
    "dummy",
    "Dummy",
    dummy_init,
    dummy_clear,
    dummy_note_on,
    dummy_note_free,
    dummy_tick,
    0,
    NULL,
    0,
    NULL
};
