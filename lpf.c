//Butterworth
#include <math.h>
#include <stdlib.h>

#include "gen.h"

struct lpf_data_s {
    double invb0, b1, b2;
};
typedef struct lpf_data_s *lpf_data_ptr;
typedef struct lpf_data_s lpf_data_t[1];

struct lpf_history_s {
    double x1, x2, y1, y2;
};
typedef struct lpf_history_s lpf_history_t[1];
typedef struct lpf_history_s *lpf_history_ptr;

void lpf_init(gen_ptr g)
{
    lpf_data_ptr p;
    g->data = malloc(sizeof(lpf_data_t));
    p = (lpf_data_ptr) g->data;
}

static inline void compute_taps(lpf_data_ptr p, double cutoff, double q)
{
    double b2, b1;

    if (cutoff >= nyquist) {
	b2 = 0.0;
    } else {
	if (cutoff < 20.0) {
	    b2 = 1.0 / tan(M_PI * 20.0 * inv_samprate);
	} else {
	    b2 = 1.0 / tan(M_PI * cutoff * inv_samprate);
	}
    }
    b1 = b2 / double_clip(q, 0.01, 200.0);
    b2 = b2 * b2;
    p->invb0 = 1.0 / (b1 + b2 + 1.0);
    p->b1 = 2.0 - 2.0 * b2;
    p->b2 = b2 - b1 + 1.0;
}

void lpf_clear(gen_ptr g)
{
    free(g->data);
}

void *lpf_note_on()
{
    lpf_history_ptr p = malloc(sizeof(lpf_history_t));
    p->x1 = 0.0;
    p->x2 = 0.0;
    p->y1 = 0.0;
    p->y2 = 0.0;
    return p;
}

void lpf_note_free(void *data)
{
    free(data);
}

double lpf_tick(gen_t g, gen_data_ptr gd, double *value)
{
    lpf_history_ptr h = (lpf_history_ptr) gd->data;
    double y;
    lpf_data_ptr p = (lpf_data_ptr) g->data;
    double x = value[0];

    compute_taps(p, value[1], value[2]);
    y = p->invb0 * (x + h->x1 * 2.0 + h->x2 - h->y1 * p->b1 - h->y2 * p->b2);
    h->x2 = h->x1;
    h->x1 = x;
    h->y2 = h->y1;
    h->y1 = y;
    return y;
}

char *lpf_port_list[] = { "input", "cutoff", "resonance" };

struct gen_info_s lpf_info = {
    "butterlpf",
    "Butterworth Lowpass",
    lpf_init,
    lpf_clear,
    lpf_note_on,
    lpf_note_free,
    lpf_tick,
    3,
    lpf_port_list,
    0,
    NULL,
};
