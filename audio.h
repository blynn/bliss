#ifndef AUDIO_H
#define AUDIO_H

enum {
    samprate = 44100,
};
void audio_init();
void audio_buffer();
void audio_set_ticker(double (*tickfn)());

#endif //AUDIO_H
