#include <stdlib.h>
#include <math.h>

#include "gen.h"

struct history_s {
    double hist[44100 + 1];
    int i;
    int age;
};
typedef struct history_s history_t[1];
typedef struct history_s *history_ptr;

void *delay_note_on()
{
    history_ptr p = (history_ptr) malloc(sizeof(history_t));
    p->i = 0;
    p->age = 0;
    return (void *) p;
}

void delay_note_free(void *data)
{
    free(data);
}

double delay_tick(gen_t g, gen_data_ptr gd, double *value)
{
    history_ptr p = (history_ptr) gd->data;
    if (!p->i) p->hist[44100] = value[0];
    p->hist[p->i] = value[0];
    p->i++;
    if (p->i >= 44100) p->i = 0;
    if (p->age < 44100) {
	p->age++;
	return 0.0;
    }
    return p->hist[p->i + 1];
}

void delay_init(gen_ptr g)
{
}

void delay_clear(gen_ptr g)
{
}

char *delay_port_list[] = { "input", NULL };
param_ptr delay_param_list[] = { NULL };

struct gen_info_s delay_info = {
    "delay",
    "Delay",
    delay_init,
    delay_clear,
    delay_note_on,
    delay_note_free,
    delay_tick,
    1,
    delay_port_list,
    0,
    delay_param_list,
};
