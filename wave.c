#include <stdlib.h>
#include "wave.h"
#include "util.h"

void wave_init(wave_ptr w)
{
    w->volume = 0.0;
    w->sample_count = 0;
    w->data_length = 0;
    w->data = NULL;
}

wave_ptr wave_new()
{
    wave_ptr w = (wave_ptr) malloc(sizeof(wave_t));
    wave_init(w);
    return w;
}

void wave_put_root_note(wave_ptr w, int n)
{
    w->root_note = n;
    w->freq = note_to_freq(n);
    w->recipfreq = 1 / w->freq;
}

void wave_clear(wave_ptr w)
{
    if (w->data) free(w->data);
}
