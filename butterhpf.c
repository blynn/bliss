//Butterworth Highpass
//with resonance [B-J]
#include <math.h>
#include <stdlib.h>

#include "gen.h"

struct butterhpf_data_s {
    double invb0, b1, b2;
};
typedef struct butterhpf_data_s *butterhpf_data_ptr;
typedef struct butterhpf_data_s butterhpf_data_t[1];

struct butterhpf_history_s {
    double x1, x2, y1, y2;
};
typedef struct butterhpf_history_s butterhpf_history_t[1];
typedef struct butterhpf_history_s *butterhpf_history_ptr;

void butterhpf_init(gen_ptr g)
{
    butterhpf_data_ptr p;
    g->data = malloc(sizeof(butterhpf_data_t));
    p = (butterhpf_data_ptr) g->data;
}

static inline void compute_taps(butterhpf_data_ptr p, double cutoff, double q)
{
    double b2, b1;

    if (cutoff >= nyquist) {
	//TODO: tan pi/2 = inf
	b2 = 1.0;
    } else {
	if (cutoff < 20.0) {
	    b2 = tan(M_PI * 20.0 * inv_samprate);
	} else {
	    b2 = tan(M_PI * cutoff * inv_samprate);
	}
    }
    b1 = b2 / double_clip(q, 0.01, 200.0);
    b2 = b2 * b2;
    p->invb0 = 1.0 / (b1 + b2 + 1.0);
    p->b1 = 2.0 * b2 - 2.0;
    p->b2 = b2 - b1 + 1.0;
}

void butterhpf_clear(gen_ptr g)
{
    free(g->data);
}

void *butterhpf_note_on()
{
    butterhpf_history_ptr p = malloc(sizeof(butterhpf_history_t));
    p->x1 = 0.0;
    p->x2 = 0.0;
    p->y1 = 0.0;
    p->y2 = 0.0;
    return p;
}

void butterhpf_note_free(void *data)
{
    free(data);
}

double butterhpf_tick(gen_t g, gen_data_ptr gd, double *value)
{
    butterhpf_history_ptr h = (butterhpf_history_ptr) gd->data;
    double y;
    butterhpf_data_ptr p = (butterhpf_data_ptr) g->data;
    double x = value[0];

    compute_taps(p, value[1], value[2]);
    y = p->invb0 * (x - h->x1 * 2.0 + h->x2 - h->y1 * p->b1 - h->y2 * p->b2);
    h->x2 = h->x1;
    h->x1 = x;
    h->y2 = h->y1;
    h->y1 = y;
    return y;
}

char *butterhpf_port_list[] = { "input", "cutoff", "resonance" };

struct gen_info_s butterhpf_info = {
    "butterhpf",
    "Butterworth Highpass",
    butterhpf_init,
    butterhpf_clear,
    butterhpf_note_on,
    butterhpf_note_free,
    butterhpf_tick,
    3,
    butterhpf_port_list,
    0,
    NULL,
};
