#include <SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "note.h"
#include "ins.h"
#include "audio.h"

#include "midi.h"

ins_t simple;

note_ptr keyboard[128];

static void midi_note_on(int noteno, int vel)
{
    SDL_LockAudio();
    keyboard[noteno] = ins_note_on(simple, noteno, ((double) vel) / 127.0);
    SDL_UnlockAudio();
}

static void midi_note_off(int noteno)
{
    ins_note_off(keyboard[noteno]);
}

static void init()
{
    if (SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0) {
	fprintf(stderr, "Can't init SDL: %s\n", SDL_GetError());
	exit(1);
    }
    atexit(SDL_Quit);

    signal(SIGINT, exit);
    signal(SIGTERM, exit);

    audio_init();
}

static double ticker()
{
    return ins_tick(simple);
}

extern struct gen_info_s osc_info;
extern struct gen_info_s lpf_info;
extern struct gen_info_s adsr_info;
extern struct gen_info_s delay_info;
extern gen_info_ptr funk_info_n(int n);

void simple_init(ins_t ins)
{
    node_ptr n0, n1, n2, n3, n4;
    node_ptr nadsr;
    ins_init(ins, "Simple");

    n0 = ins_add_gen(ins, &osc_info, "n0");
    n1 = ins_add_gen(ins, &osc_info, "n1");
    n2 = ins_add_gen(ins, &lpf_info, "n2");
    n3 = ins_add_gen(ins, funk_info_n(2), "n3");
    n4 = ins_add_gen(ins, &delay_info, "n4");
    nadsr = ins_add_gen(ins, &adsr_info, "adsr");
    set_funk_program(n3, "mul(x1, 2.0)");
    ins_connect(ins, ins->freq, n0, 0);
    //ins_connect(ins, n0, n4, 0);
    //ins_connect(ins, n4, ins->out, 0);
    ins_connect(ins, n0, n2, 0);
    ins_connect(ins, n1, n2, 0);
    set_param_by_id(n1, "shape", 2);
    ins_connect(ins, ins->freq, n3, 1);
    ins_connect(ins, n3, n1, 0);
    set_param_by_id(n2, "cutoff", 0.01);
    ins_connect(ins, n2, nadsr, 0);
    ins_connect(ins, nadsr, ins->out, 0);
}

int main(int argc, char **argv)
{
    struct midi_cb_s midicbp = {
	midi_note_on,
	midi_note_off
    };

    simple_init(simple);
    audio_set_ticker(ticker);
    init();

    SDL_PauseAudio(0);
    midi_start(&midicbp);
    {
	char c;
	c = fgetc(stdin);
    }
    midi_stop();
    SDL_PauseAudio(1);

    return 0;
}
