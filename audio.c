#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL.h>

#include "darray.h"
#include "audio.h"
#include "note.h"

static double (*audio_tick)();

static void convert_write(Uint8 *ptr, double x)
{
    int n;
    n = x * 4096;
    //if (n > 32767) n = 32767;
    //else if (n < -32767) n = -32767;
    *ptr = (Uint8) (n & 255); //lower 16 bits
    n = n >> 8;
    ptr[1] = (Uint8) (n & 255); //upper 16 bits
}

static void c_fill_audio(void *udata, Uint8 *stream, int len)
{
    Uint8 *ptr = stream;
    Uint8 *target = stream + len;

    while (ptr != target) {
	double l, r;
	l = audio_tick();
	r = l;

	convert_write(ptr, l);
	ptr += 2;
	convert_write(ptr, r);
	ptr += 2;
    }
}

void audio_set_ticker(double (*tickfn)())
{
    audio_tick = tickfn;
}

void audio_init()
{
    SDL_AudioSpec wanted;

    /* Set the audio format */
    wanted.freq = samprate;
    wanted.format = AUDIO_S16;
    wanted.channels = 2;    /* 1 = mono, 2 = stereo */
    wanted.samples = 512;  /* Good low-latency value for callback */
    wanted.callback = c_fill_audio;
    wanted.userdata = NULL;

    /* Open the audio device, forcing the desired format */
    if ( SDL_OpenAudio(&wanted, NULL) < 0 ) {
	fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
	exit(-1);
    }
}
