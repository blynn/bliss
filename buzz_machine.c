#include <stdlib.h>
#include "buzz_machine.h"
#include "util.h"

static buzz_machine_info_ptr tmpmi;

void global_param(buzz_param_ptr bp)
{
    darray_append(tmpmi->gparam, bp);
    tmpmi->gpsize++;
    if (bp->type == pt_word) tmpmi->gpsize++;
}

void track_param(buzz_param_ptr bp)
{
    darray_append(tmpmi->tparam, bp);
    tmpmi->tpsize++;
    if (bp->type == pt_word) tmpmi->tpsize++;
}

static void buzz_param_cell_init(cell_ptr c, char *text, buzz_param_ptr bp)
{
    switch(bp->type) {
	case pt_switch:
	case pt_byte:
	case pt_word:
	    cell_init_int(c, strtol(text, NULL, 16));
	    break;
	case pt_note:
	    cell_init_note(c, strtonote(text));
	    break;
    }
    //TODO: clipping
}

static void buzz_machine_cell_init(cell_ptr c, machine_ptr m, char *text, int col)
{
    buzz_machine_info_ptr buzzmi = m->mi->buzzmi;
    buzz_param_ptr bp;

    if (col < buzzmi->gparam->count) {
	bp = (buzz_param_ptr) buzzmi->gparam->item[col];
	buzz_param_cell_init(c, text, bp);
    } else {
	int i;

	i = col - buzzmi->gparam->count;
	i = i % buzzmi->tparam->count;
	bp = (buzz_param_ptr) buzzmi->tparam->item[i];
	buzz_param_cell_init(c, text, bp);
    }
}

static void buzz_machine_parse(machine_t m, cell_t c, int col)
{
    buzz_machine_info_ptr buzzmi = m->mi->buzzmi;
    buzz_param_ptr bp;
    int n = c->data.i;

    if (col < buzzmi->gparam->count) {
	bp = (buzz_param_ptr) buzzmi->gparam->item[col];

	m->buzz_state[col] = n;
	bp->func(m, -1, n);
    } else {
	int track;
	int i;

	i = col - buzzmi->gparam->count;
	track = i / buzzmi->tparam->count;
	i = i % buzzmi->tparam->count;
	bp = (buzz_param_ptr) buzzmi->tparam->item[i];

	m->buzz_state[col] = n;
	bp->func(m, track, n);
    }
}

static void buzz_machine_init(machine_t m)
{
    buzz_machine_info_ptr buzzmi = m->mi->buzzmi;
    int i, j, col;
    m->buzz_state = (int *) malloc(sizeof(int) *
	    (buzzmi->track_max * buzzmi->tparam->count + buzzmi->gparam->count));
    col = 0;
    for (i=0; i<buzzmi->gparam->count; i++) {
	buzz_param_ptr bp = (buzz_param_ptr) buzzmi->gparam->item[i];
	m->buzz_state[col] = bp->defval;
	col++;
    }
    for (j=0; j<buzzmi->track_max; j++) {
	for (i=0; i<buzzmi->tparam->count; i++) {
	    buzz_param_ptr bp = (buzz_param_ptr) buzzmi->tparam->item[i];
	    m->buzz_state[col] = bp->defval;
	    col++;
	}
    }
    buzzmi->init(m);
}

static void buzz_machine_work(machine_t m, double *l, double *r)
{
    double bl, br;

    bl = *l * 32768;
    br = *r * 32768;

    m->mi->buzzmi->work(m, &bl, &br);

    *l = bl / 32768;
    *r = br / 32768;
}

static void buzz_machine_clear(machine_t m)
{
    buzz_machine_info_ptr buzzmi = m->mi->buzzmi;

    buzzmi->clear(m);

    free(m->buzz_state);
}

static void buzz_machine_print_state(machine_t m, FILE *fp)
{
    //TODO
}

void machine_info_init(machine_info_ptr mi)
{
    tmpmi = (buzz_machine_info_ptr) malloc(sizeof(buzz_machine_info_t));
    buzz_machine_info_init(tmpmi);

    tmpmi->tpsize = 0;
    tmpmi->gpsize = 0;
    darray_init(tmpmi->tparam);
    darray_init(tmpmi->gparam);
    buzz_param_init();

    mi->buzzmi = tmpmi;
    mi->type = tmpmi->type;
    mi->id = tmpmi->id;
    mi->name = tmpmi->name;
    mi->parse = buzz_machine_parse;
    mi->cell_init = buzz_machine_cell_init;
    mi->init = buzz_machine_init;
    mi->clear = buzz_machine_clear;
    mi->work = buzz_machine_work;
    mi->tick = tmpmi->tick;
    mi->print_state = buzz_machine_print_state;
}
