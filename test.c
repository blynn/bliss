#include <SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "note.h"
#include "voice.h"
#include "audio.h"

#include "midi.h"

voice_t simple;

static void midi_note_on(int noteno, int vel)
{
    SDL_LockAudio();
    voice_note_on(simple, noteno, ((double) vel) / 127.0);
    SDL_UnlockAudio();
}

static void midi_note_off(int noteno)
{
    voice_note_off(simple, noteno);
}

static void init()
{
    if (SDL_Init(SDL_INIT_AUDIO|SDL_INIT_TIMER) < 0) {
	fprintf(stderr, "Can't init SDL: %s\n", SDL_GetError());
	exit(1);
    }
    atexit(SDL_Quit);

    signal(SIGINT, exit);
    signal(SIGTERM, exit);

    audio_init(512);
}

static double ticker()
{
    return voice_tick(simple);
}

extern struct gen_info_s osc_info;
extern struct gen_info_s butterlpf_info;
extern struct gen_info_s adsr_info;
extern struct gen_info_s delay_info;
extern gen_info_ptr funk_info_n(int n);

void simple_init(voice_t ins)
{
    node_ptr nosc1, nosc2;
    node_ptr nf;
    node_ptr nadsr;
    voice_init(ins, "Simple");

    nosc1 = voice_add_gen(ins, &osc_info, "osc0");
    nosc2 = voice_add_gen(ins, &osc_info, "osc1");
    nadsr = voice_add_gen(ins, &adsr_info, "adsr");
    nf = voice_add_gen(ins, funk_info_n(1), "f");
    set_funk_program(nf, "x1 * 2.0");
    voice_connect(ins, ins->freq, nosc1, 0);
    voice_connect(ins, ins->freq, nf, 0);
    set_param_by_id(nosc1, "waveform", 2.0);
    voice_connect(ins, nf, nosc2, 0);
    voice_connect(ins, nosc1, nadsr, 0);
    voice_connect(ins, nosc2, nadsr, 0);
    voice_connect(ins, nadsr, ins->out, 0);
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
