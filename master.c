#include <stdlib.h>
#include "machine.h"
#include "song.h"

enum {
    default_bpm = 126,
    default_tpb = 4,
};

struct master_data_s {
    int volume;
    int bpm;
    int tpb;
};
typedef struct master_data_s *master_data_ptr;

static void master_work(machine_t m, double *l, double *r)
{
}

static void master_init(machine_t m)
{
    master_data_ptr p;

    song_put_bpm_tpb(m->song, default_bpm, default_tpb);
    m->data = malloc(sizeof(struct master_data_s));
    p = (master_data_ptr) m->data;
    p->bpm = default_bpm;
    p->tpb = default_tpb;
}

static void master_clear(machine_t m)
{
    free(m->data);
}

static void master_parse(machine_t m, cell_t c, int col)
{
    master_data_ptr p = (master_data_ptr) m->data;

    if (c->type == t_string) {
	switch (c->data.s[0]) {
	    case 'b':
		p->bpm = strtol(&c->data.s[1], NULL, 16);
		song_put_bpm(m->song, p->bpm);
		break;
	    case 't':
		p->tpb = strtol(&c->data.s[1], NULL, 16);
		song_put_tpb(m->song, p->tpb);
		break;
	}
    }
}

static void master_tick(machine_t m)
{
}

static void master_print_state(machine_t m, FILE *fp)
{
    master_data_ptr p = (master_data_ptr) m->data;
    fprintf(fp, "b%x\n", p->bpm);
    fprintf(fp, "t%x\n", p->tpb);
}

void master_machine_info_init(machine_info_ptr mi)
{
    mi->type = machine_master;
    mi->id = "Master";
    mi->name = "Master";
    mi->init = master_init;
    mi->clear = master_clear;
    mi->work = master_work;
    mi->parse = master_parse;
    mi->tick = master_tick;
    mi->print_state = master_print_state;
}
