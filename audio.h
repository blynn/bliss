#ifndef AUDIO_H
#define AUDIO_H

enum {
    devsamprate = 44100,
};
void audio_init(int latency);
void audio_set_ticker(double (*tickfn)());
void audio_start();
void audio_stop();

#endif //AUDIO_H
