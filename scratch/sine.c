#include <SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>

#include "audio.h"

static void init()
{
    if (SDL_Init(SDL_INIT_AUDIO|SDL_INIT_TIMER) < 0) {
	fprintf(stderr, "Can't init SDL: %s\n", SDL_GetError());
	exit(1);
    }
    atexit(SDL_Quit);

    signal(SIGINT, exit);
    signal(SIGTERM, exit);

    audio_init();
}

static double phase = 0.0;

static double ticker()
{
    phase += 440.0 / 44100.0;
    if (phase > 1.0) phase -= 1.0;
    return sin(2.0 * M_PI * phase);
}

int main(int argc, char **argv)
{
    printf("Attempting to play 440Hz for 2s\n");
    audio_set_ticker(ticker);
    init();

    audio_start();
    SDL_Delay(2000);
    audio_stop();

    return 0;
}
