#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "machine.h"

struct sine_data_s {
    double f;
    int i;
};
typedef struct sine_data_s *sine_data_ptr;

static void sine_work(machine_t m, double *l, double *r)
{
    sine_data_ptr p = (sine_data_ptr) m->data;
    int i = p->i;

    *l = sin(p->f * i * 2.0 * M_PI / 44100.0) * 0.2;
    *r = *l;
    i = (i + 1) % samprate;
    p->i = i;
}

static void sine_init(machine_t m)
{
    sine_data_ptr p;
    m->data = malloc(sizeof(struct sine_data_s));
    p = (sine_data_ptr) m->data;
    p->f = 220;
    p->i = 0;
}

static void sine_clear(machine_t m)
{
    free(m->data);
}

static void sine_parse(machine_t m, char *cmd, int col)
{
    sine_data_ptr p = (sine_data_ptr) m->data;

    sscanf(cmd, "%lf", &p->f);
}

struct machine_info_s machine_info = {
    machine_generator,
    "Sine Demo Machine",
    "Sine",
    sine_init,
    sine_clear,
    sine_work,
    sine_parse
};
