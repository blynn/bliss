#include <math.h>
#include "machine.h"

static void master_work(machine_t m, double *l, double *r)
{
}

static void master_init(machine_t m)
{
}

static void master_clear(machine_t m)
{
}

static void master_parse(machine_t m, char *s, int col)
{
}

static void master_tick(machine_t m)
{
}

void machine_info_init(machine_info_ptr mi)
{
    mi->type = machine_master;
    mi->id = "Master";
    mi->name = "Master";
    mi->init = master_init;
    mi->clear = master_clear;
    mi->work = master_work;
    mi->parse = master_parse;
    mi->tick = master_tick;
}
