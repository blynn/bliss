#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "machine.h"
#include "util.h"

void machine_init(machine_t m, machine_info_ptr mi, song_ptr s, char *id)
{
    m->song = s;
    m->mi = mi;
    mi->init(m);
    m->id = strclone(id);
    darray_init(m->out);
    darray_init(m->in);
    m->track = track_new(m);
    darray_init(m->pattern);
}

void machine_clear(machine_t m)
{
    int i, n;

    darray_clear(m->out);
    darray_clear(m->in);
    n = m->pattern->count;
    for (i=0; i<n; i++) {
	pattern_ptr p = (pattern_ptr) m->pattern->item[i];
	pattern_clear(p);
	free(p);
    }
    darray_clear(m->pattern);
    track_clear(m->track);
    free(m->track);
    m->mi->clear(m);
}

machine_ptr machine_new(machine_info_t mi, song_ptr s, char *id)
{
    machine_ptr m = (machine_ptr) malloc(sizeof(struct machine_s));
    machine_init(m, mi, s, id);
    return m;
}

edge_ptr edge_new(machine_t src, machine_t dst)
{
    edge_ptr e;
    e = (edge_ptr) malloc(sizeof(struct edge_s));
    e->src = src;
    e->dst = dst;
    return e;
}

void edge_clear(edge_ptr e)
{
}

pattern_ptr machine_pattern_at(machine_ptr m, char *id)
{
    int i, n;

    n = m->pattern->count;
    for (i=0; i<n; i++) {
	pattern_ptr p = (pattern_ptr) m->pattern->item[i];
	if (!strcmp(p->id, id)) {
	    return p;
	}
    }
    return NULL;
}

void machine_next_sample(machine_ptr m, double *l, double *r)
{
    int i, n;
    edge_ptr e;
    if (m->visited) {
	*l = m->l;
	*r = m->r;
	return;
    }

    //(linearly) combine all inputs
    m->l = m->r = 0;
    m->visited = 1;
    n = m->in->count;
    for (i=0; i<n; i++) {
	double d1, d2;
	e = m->in->item[i];
	machine_next_sample(e->src, &d1, &d2);
	m->l += d1;
	m->r += d2;
    }
    m->mi->work(m, &m->l, &m->r);
    *l = m->l;
    *r = m->r;
}

void machine_parse(machine_t m, cell_t c, int col)
{
    m->mi->parse(m, c, col);
}

void machine_tick(machine_t m)
{
    m->mi->tick(m);
}

static char *machine_new_pattern_name(machine_t m)
{
    static char s[4];
    int count = 0;

    for(;;) {
	sprintf(s, "%02d", count++);
	if (!machine_pattern_at(m, s)) return(strclone(s));
    }
}

pattern_ptr machine_create_pattern_auto_id(machine_ptr m)
{
    pattern_ptr p;
    char *s;

    s = machine_new_pattern_name(m);
    p = pattern_new(m);
    p->id = s;
    return p;
}

pattern_ptr machine_create_pattern(machine_ptr m, char *id)
{
    pattern_ptr p;

    p = pattern_new(m);
    p->id = strclone(id);
    return p;
}

void machine_cell_init(cell_ptr c, machine_ptr m, char *text, int col)
{
    m->mi->cell_init(c, m, text, col);
}

void machine_print_state(machine_ptr m, FILE *fp)
{
    m->mi->print_state(m, fp);
}
