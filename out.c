#include <stdlib.h>

#include "gen.h"

void *out_note_on()
{
    return NULL;
}

void out_note_free(void *data)
{
}

double out_tick(gen_t g, void *data, double *value)
{
    return value[0];
}

void out_init(gen_ptr g)
{
}

void out_clear(gen_ptr g)
{
}

char *out_port_list[] = { "input" };

struct gen_info_s out_info = {
    "out",
    out_init,
    out_clear,
    out_note_on,
    out_note_free,
    out_tick,
    1,
    out_port_list,
    0,
    NULL
};
