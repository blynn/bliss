#include <math.h>
#include <stdlib.h>
#include "machine.h"
#include "darray.h"

struct note_s {
    double f;
    int i;
    int ttl;
};
typedef struct note_s *note_ptr;

struct sine_data_s {
    darray_t note;
};
typedef struct sine_data_s *sine_data_ptr;

static void sine_work(machine_t m, double *l, double *r)
{
    sine_data_ptr p = (sine_data_ptr) m->data;
    int i;
    for (i=0; i<p->note->count; i++) {
	note_ptr note = (note_ptr) p->note->item[i];

	*l += sin(note->f * note->i * 2.0 * M_PI / 44100.0) * 0.2;
	note->i = (note->i + 1) % samprate;

	note->ttl--;
	if (note->ttl <= 0) darray_remove(p->note, note);
    }
    *r = *l;
}

static void new_note(sine_data_ptr p, double f)
{
    note_ptr n = (note_ptr) malloc(sizeof(struct note_s));
    n->f = f;
    n->ttl = 20000; //~ 0.5 sec
    n->i = 0;
    darray_append(p->note, n);
}

static void sine_init(machine_t m)
{
    sine_data_ptr p;
    m->data = malloc(sizeof(struct sine_data_s));
    p = (sine_data_ptr) m->data;
    darray_init(p->note);
}

static void sine_clear(machine_t m)
{
    int i, n;
    sine_data_ptr p = (sine_data_ptr) m->data;
    n = p->note->count;
    for(i=0; i<n; i++) {
	free(p->note->item[i]);
    }
    darray_clear(p->note);
    free(m->data);
}

static void sine_parse(machine_t m, cell_t c, int col)
{
    sine_data_ptr p = (sine_data_ptr) m->data;
    if (c->type == t_int) new_note(p, c->data.i);
    else if (c->type == t_string) new_note(p, atof(c->data.s));
}

void machine_info_init(machine_info_ptr mi)
{
    mi->type = machine_generator;
    mi->id = "Sine Demo Machine";
    mi->name = "Sine";
    mi->init = sine_init;
    mi->clear = sine_clear;
    mi->work = sine_work;
    mi->parse = sine_parse;
}
