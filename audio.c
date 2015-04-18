#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL.h>

#include "darray.h"
#include "audio.h"
#include "note.h"

static double (*audio_tick)();

static inline void convert_write(Uint8 *ptr, double x) {
  int n;
  n = x * 8192;
  //if (n > 32767) n = 32767;
  //else if (n < -32767) n = -32767;

  *ptr = (Uint8) (n & 255); //lower 16 bits
  n = n >> 8;
  ptr[1] = (Uint8) (n & 255); //upper 16 bits
}

static void c_fill_audio(void *udata, Uint8 *p, int len) {
  Uint8* end = p + len;
  while (p < end) {
    double l = audio_tick(), r = l;

    convert_write(p, l);
    p += 2;
    convert_write(p, r);
    p += 2;
  }
}

void audio_set_ticker(double (*tickfn)()) { audio_tick = tickfn; }

void audio_init(int latency) {
  SDL_AudioSpec wanted;

  // Set the audio format
  wanted.freq = devsamprate;
  wanted.format = AUDIO_S16;
  wanted.channels = 2;  // 1 = mono, 2 = stereo
  wanted.samples = latency;
  wanted.callback = c_fill_audio;
  wanted.userdata = NULL;

  // Open the audio device, forcing the desired format
  if (SDL_OpenAudio(&wanted, NULL) < 0) {
    fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
    exit(-1);
  }
}

void audio_stop() { SDL_PauseAudio(1); }

void audio_start() { SDL_PauseAudio(0); }
