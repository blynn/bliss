#include <string.h>
#include <stdlib.h>
#include "machine.h"
#include "mlist.h"
#include "darray.h"
#include "util.h"

struct state_s {
    int visited;
    double *data;
    void *p;
};

typedef struct state_s *state_ptr;

struct note_s {
    state_ptr state;
    int ttl;
    int i_out0;
};
typedef struct note_s *note_ptr;

static void note_free(machine_info_ptr mi, darray_t a, note_ptr p)
{
    int i, n;
    darray_remove(a, p);

    n = mi->unit->count;
    for (i=0; i<n; i++) {
	unit_ptr u = (unit_ptr) mi->unit->item[i];
	if (u->type == ut_plugin) {
	    u->ui->note_free(p->state[i].p);
	}
	free(p->state[i].data);
    }
    free(p->state);
    free(p);
}

static void unit_run(machine_info_ptr mi, note_ptr p, int index)
{
    unit_ptr u = (unit_ptr) mi->unit->item[index];
    state_ptr sp = &p->state[index];
    int i, n;
    int pc;

    if (sp->visited) return;
    sp->visited = 1;

    switch (u->type) {
	case ut_stream:
	    pc = 1;
	    break;
	case ut_plugin:
	    pc = u->ui->port_count;
	    break;
	case ut_param:
	    return;
	default:
	    fprintf(stderr, "unhandled\n");
	    return;
    }

    for (i=0; i<pc; i++) sp->data[i] = 0.0;

    n = u->in->count;
    for (i=0; i<n; i++) {
	unit_edge_ptr e = (unit_edge_ptr) u->in->item[i];
	unit_run(mi, p, e->src->index);
	sp->data[e->dstport] += p->state[e->src->index].data[e->srcport];
    }
    if (u->type == ut_plugin) {
	u->ui->run(sp->p);
    }
}

static void note_run(machine_info_ptr mi, note_ptr p)
{
    int i, n;
    n = mi->unit->count;
    for (i=0; i<n; i++) p->state[i].visited = 0;
    unit_run(mi, p, p->i_out0);
}

static void bmachine_work(machine_t m, double *l, double *r)
{
    darray_ptr a = (darray_ptr) m->data;
    int i;

    for (i=0; i<a->count;) {
	note_ptr p = (note_ptr) a->item[i];
	if (p->ttl) {
	    note_run(m->mi, p);
	    *l += p->state[p->i_out0].data[0];
	    //TODO: change this to i_out1
	    *r += p->state[p->i_out0].data[0];
	    p->ttl--;
	    if (!p->ttl) {
		note_free(m->mi, a, p);
	    } else {
		i++;
	    }
	}
    }
}

static int index_stream(machine_info_ptr mi, char *s)
{
    int i, n;
    n = mi->unit->count;
    for (i=0; i<n; i++) {
	unit_ptr u = (unit_ptr) mi->unit->item[i];
	if (!strcmp(s, u->id)) {
	    return i;
	}
    }
    return -1;
}

static int index_param(machine_info_ptr mi, char *s)
{
    int i, n;
    n = mi->unit->count;
    for (i=0; i<n; i++) {
	unit_ptr u = (unit_ptr) mi->unit->item[i];
	if (!strcmp(s, u->id)) {
	    return i;
	}
    }
    return -1;
}

static void new_note_freq(machine_t m, darray_ptr a, double f)
{
    int i, j, n, pc;
    unit_ptr u;
    note_ptr p = (note_ptr) malloc(sizeof(struct note_s));

    n = m->mi->unit->count;
    p->state = (state_ptr) malloc(n * sizeof(struct state_s));
    for (i=0; i<n; i++) {
	u = (unit_ptr) m->mi->unit->item[i];
	switch (u->type) {
	    case ut_param:
	    case ut_stream:
		p->state[i].data = (double *) calloc(1, sizeof(double));
		p->state[i].p = NULL;
		break;
	    case ut_plugin:
		pc = u->ui->port_count;
		p->state[i].data = (double *) calloc(pc, sizeof(double));
		p->state[i].p = u->ui->note_new();
		for (j=0; j<pc; j++) {
		    u->ui->connect_port(p->state[i].p, j, &p->state[i].data[j]);
		}
		break;
	    default:
		fprintf(stderr, "new_note_freq: bug!\n");
		break;
	}
    }
    darray_append(a, p);
    p->ttl = 20000;

    p->i_out0 = index_stream(m->mi, "out0");
    i = index_param(m->mi, "note");
    p->state[i].data[0] = f;
}

static void bmachine_init(machine_t m)
{
    darray_ptr a;
    m->data = malloc(sizeof(darray_t));
    a = (darray_ptr) m->data;
    darray_init(a);
}

static void bmachine_clear(machine_t m)
{
    darray_ptr a = (darray_ptr) m->data;
    //TODO: free notes if not finished playing
    darray_clear(a);
    free(m->data);
}

static void bmachine_parse(machine_t m, cell_t c, int col)
{
    darray_ptr a = (darray_ptr) m->data;
    if (c->type == t_note) new_note_freq(m, a, note_to_freq(c->data.i));
}

static void bmachine_cell_init(cell_t c, machine_t m, char *text, int col)
{
    char *s = strclone(text);
    char *arg = strchr(s, '=');
    if (!arg) {
	cell_init_note(c, strtonote(s));
    } else {
	*arg = 0;
	arg++;
	cell_init_assign(c, s, atof(arg));
    }
    free(s);
}

void bmachine_info_init(machine_info_ptr mi)
{
    mi->is_bliss = 1;
    mi->init = bmachine_init;
    mi->clear = bmachine_clear;
    mi->work = bmachine_work;
    mi->parse = bmachine_parse;
    mi->cell_init = bmachine_cell_init;
    darray_init(mi->unit);
    darray_init(mi->unit_edge);
}

machine_info_ptr bmachine_new()
{
    machine_info_ptr mi = machine_info_new();
    bmachine_info_init(mi);
    return mi;
}

unit_ptr bmachine_unit_at(machine_info_ptr mi, char *id)
{
    int i, n;
    darray_ptr a;
    unit_ptr m;

    a = mi->unit;
    n = a->count;
    for (i=0; i<n; i++) {
	m = (unit_ptr) a->item[i];
	if (!strcmp(m->id, id)) {
	    return m;
	}
    }
    return NULL;
}

unit_ptr bmachine_create_unit_auto_id(machine_info_ptr mi, char *gearid)
{
    unit_ptr m;
    char *id;
    char temp[10];
    int count;
    unit_info_ptr ui;

    ui = unit_info_at(gearid);
    if (!ui) return NULL;

    //work out unique name
    id = (char *) alloca(strlen(ui->name) + 10);
    strcpy(id, ui->name);
    count = 2;
    for (;;) {
	unit_ptr u;
	u = bmachine_unit_at(mi, id);
	if (!u) break;
	strcpy(id, ui->name);
	sprintf(temp, "%d", count);
	strcat(id, temp);
	count++;
    }

    m = unit_new(ui, id);
    m->index = mi->unit->count;
    darray_append(mi->unit, m);
    return m;
}

void bmachine_add_unit(machine_info_ptr mi, unit_ptr u)
{
    u->index = mi->unit->count;
    darray_append(mi->unit, u);
}

void bmachine_remove_unit(machine_info_ptr mi, unit_ptr u)
{
    //TODO: replace with more efficient algorithm?
    int i, n;
    darray_remove(mi->unit, u);
    n = mi->unit->count;
    for (i=0; i<n; i++) {
	unit_ptr u1 = (unit_ptr) mi->unit->item[i];
	u1->index = i;
    }
}
