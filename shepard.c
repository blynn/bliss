#include <stdlib.h>
#include <math.h>

#include "gen.h"

#define LN2 0.69314718

struct shepard_data_s {
    double invlog;
};

typedef struct shepard_data_s shepard_data_t[1];
typedef struct shepard_data_s *shepard_data_ptr;

void *shepard_note_on()
{
    double *res = (double *) malloc(sizeof(double));
    *res = 0.0;
    return (void *) res;
}

void shepard_note_free(void *data)
{
    free(data);
}

double shepard_tick(gen_t g, gen_data_ptr gd, double *value)
{
    double res;
    shepard_data_ptr p = g->data;
    double *dp;
    double y, x;
    double f = value[0];

    dp = (double *) gd->data;
    if (f == 0.0) return 0.0;
    while (f > 40.0) f *= 0.5;
    *dp = (*dp) + f * inv_samprate;
    if (*dp > 1.0) *dp -= 1.0;
    x = log(f) * p->invlog;
    y = cos(*dp * 2.0 * M_PI);
    res = (0.5 - 0.5 * cos(x * 2.0 * M_PI)) * y;
    for (;;) {
	x += LN2 * p->invlog;
	if (x > 1.0) break;
	//double angle cosine formula
	y = 2.0 * y * y - 1.0;
	res += (0.5 - 0.5 * cos(x * 2.0 * M_PI)) * y;
    }
    return res;
}

void shepard_init(gen_ptr g)
{
    g->data = malloc(sizeof(shepard_data_t));
    ((shepard_data_ptr) g->data)->invlog = 1.0 / log(nyquist);
}

void shepard_clear(gen_ptr g)
{
    free(g->data);
}

static char *shepard_port_list[] = { "freq" };

struct gen_info_s shepard_info = {
    "shepard",
    "Shepard Tone",
    shepard_init,
    shepard_clear,
    shepard_note_on,
    shepard_note_free,
    shepard_tick,
    1,
    shepard_port_list,
    0,
    NULL,
};
