//Two pole filter
//
//                1
//H(z) = -------------------
//       1 - s z^-1 + p z^-2
//
//where s = sum of roots, p = product of roots
//for frequency f, radius r, roots are re^if
//(f is scaled to lie between 0 and pi)
#include <math.h>
#include <stdlib.h>

#include "gen.h"

struct twopole_history_s {
    double y1;
    double y2;
};
typedef struct twopole_history_s twopole_history_t[1];
typedef struct twopole_history_s *twopole_history_ptr;

void twopole_init(gen_ptr g)
{
}

void twopole_clear(gen_ptr g)
{
}

void *twopole_note_on()
{
    twopole_history_ptr p = malloc(sizeof(twopole_history_t));
    p->y1 = 0.0;
    p->y2 = 0.0;
    return p;
}

void twopole_note_free(void *data)
{
    free(data);
}

double twopole_tick(gen_t g, gen_data_ptr gd, double *value)
{
    twopole_history_ptr h = (twopole_history_ptr) gd->data;
    double y;
    double theta = M_PI * value[1] * inv_nyquist;
    double r = double_clip(value[2], -1.0, 1.0);
    double b2 = r * r;
    double b1 = 2.0 * r * cos(theta);

    y = value[0] + b1 * h->y1 - b2 * h->y2;;
    h->y2 = h->y1;
    h->y1 = y;
    return y;
}

char *twopole_port_list[] = { "input", "freq", "radius" };

struct gen_info_s twopole_info = {
    "twopole",
    "Two Pole Filter",
    twopole_init,
    twopole_clear,
    twopole_note_on,
    twopole_note_free,
    twopole_tick,
    3,
    twopole_port_list,
    0,
    NULL,
};
