#include "machine.h"
#include "buzz_machine.h"

static void handle_cutoff(int track, int val)
{
    printf("%d cutoff = %d\n", track, val);
}

static void handle_unknown(int track, int val)
{
}

static struct buzz_param_s param_cutoff = {
    pt_byte,
    "Cutoff",
    "Cutoff",
    1,
    255,
    0,
    0,
    0,
    handle_cutoff,
};

static struct buzz_param_s param_unk = {
    pt_byte,
    "Unknown",
    "Unknown",
    0,
    0,
    0,
    0,
    0,
    handle_unknown,
};

static buzz_machine_info_t buzzm;

void add_track_param(buzz_param_ptr bp)
{
    darray_append(buzzm->tparam, bp);
}

void buzz_machine_init()
{
    darray_init(buzzm->tparam);
    darray_init(buzzm->gparam);
}

void buzz_param_init()
{
    add_track_param(&param_cutoff);
    add_track_param(&param_unk);
    add_track_param(&param_unk);
    add_track_param(&param_unk);
    add_track_param(&param_unk);
    add_track_param(&param_unk);
    add_track_param(&param_unk);
    add_track_param(&param_unk);
    add_track_param(&param_unk);
    /*
    add_track_param(pt_byte, "Resonance", "Resonance", 1, 255, 0, 0, 0);
    add_track_param(pt_byte, "Unknown", "Unknown", 0, 0, 0, 0, 0);
    add_track_param(pt_byte, "Unknown", "Unknown", 0, 0, 0, 0, 0);
    add_track_param(pt_byte, "Unknown", "Unknown", 0, 0, 0, 0, 0);
    add_track_param(pt_note, "Note", "Note", NOTE_MIN, NOTE_MAX, NOTE_NO, 0, 0);
    add_track_param(pt_byte, "Volume", "Volume Level", 0, 254, 255, 0, 255);
    add_track_param(pt_byte, "Length", "Length in ticks", 1, 128, 0, 0, 0);
    add_track_param(pt_byte, "Unknown", "Unknown", 0, 0, 0, 0, 0);
    */
}

static void abass2_init(machine_t m)
{
}

static void abass2_clear(machine_t m)
{
}

static void abass2_tick(machine_t m)
{
}

static void abass2_work(machine_t m, double *l, double *r)
{
}

static void buzz_machine_parse(machine_t m, char *cmd, int col)
{
    if (col < buzzm->gparam->count) {
    } else {
	buzz_param_ptr bp;
	int track;
	int i;

	i = col - buzzm->gparam->count;
	track = i / buzzm->tparam->count;
	i = i % buzzm->tparam->count;
	bp = (buzz_param_ptr) buzzm->tparam->item[i];

	bp->func(track, 5);
    }
}

void machine_info_init(machine_info_ptr mi)
{
    buzz_machine_init();
    buzz_param_init();
    mi->type = machine_generator;
    mi->id = "Alpha Bass 2";
    mi->name = "aBass2";
    mi->init = abass2_init;
    mi->clear = abass2_clear;
    mi->work = abass2_work;
    mi->parse = buzz_machine_parse;
    mi->tick = abass2_tick;
}
