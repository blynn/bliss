//One pole filter, with normalized peak gain
//
//           1
//H(z) = ----------
//       1 - r z^-1
//
//for stability, |r| < 1
//peak gain = 1/(1-r) for r > 0
//          = 1/(1+r) for r < 0
#include <math.h>
#include <stdlib.h>

#include "gen.h"

struct onepole_history_s {
    double y1;
};
typedef struct onepole_history_s onepole_history_t[1];
typedef struct onepole_history_s *onepole_history_ptr;

void onepole_init(gen_ptr g)
{
}

void onepole_clear(gen_ptr g)
{
}

void *onepole_note_on()
{
    onepole_history_ptr p = malloc(sizeof(onepole_history_t));
    p->y1 = 0.0;
    return p;
}

void onepole_note_free(void *data)
{
    free(data);
}

double onepole_tick(gen_t g, gen_data_ptr gd, double *value)
{
    onepole_history_ptr h = (onepole_history_ptr) gd->data;
    double y;
    double r = double_clip(value[1], -1.0, 1.0);
    double a;

    if (r > 0.0) {
	a = 1.0 - r;
    } else {
	a = 1.0 + r;
    }
    y = a * value[0] + r * h->y1;
    h->y1 = y;
    return y;
}

char *onepole_port_list[] = { "input", "pole" };

struct gen_info_s onepole_info = {
    "onepole",
    "One Pole Filter",
    onepole_init,
    onepole_clear,
    onepole_note_on,
    onepole_note_free,
    onepole_tick,
    2,
    onepole_port_list,
    0,
    NULL,
};
