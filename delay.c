#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "gen.h"

enum {
    buflen = 44100, //TODO: make this a param
};

struct history_s {
    double hist[buflen];
    int i;
};
typedef struct history_s history_t[1];
typedef struct history_s *history_ptr;

void *delay_note_on()
{
    history_ptr p = (history_ptr) malloc(sizeof(history_t));
    memset(p->hist, 0, sizeof(double) * buflen);
    p->i = 0;
    return (void *) p;
}

void delay_note_free(void *data)
{
    free(data);
}

double delay_tick(gen_t g, gen_data_ptr gd, double *value)
{
    int j;
    int n = value[1] * samprate;
    history_ptr p = (history_ptr) gd->data;
    p->hist[p->i] = value[0];
    p->i++;
    if (p->i == buflen) p->i = 0;
    if (n < 0) return 0.0; //can't look into the future
    j = p->i - n;
    if (j < 0) j += buflen;

    if (j < 0) return 0.0; //buffer not big enough for requested delay

    return p->hist[j];
}

void delay_init(gen_ptr g)
{
}

void delay_clear(gen_ptr g)
{
}

char *delay_port_list[] = { "input", "delay" };

struct gen_info_s delay_info = {
    "delay",
    "Delay",
    delay_init,
    delay_clear,
    delay_note_on,
    delay_note_free,
    delay_tick,
    2,
    delay_port_list,
    0,
    NULL,
};
