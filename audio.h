#ifndef AUDIO_H
#define AUDIO_H

struct song_s;

enum {
    samprate = 44100,
};
void audio_init();
void audio_play();
void audio_pause();
void audio_put_song(struct song_s *);
void audio_buffer();

#endif //AUDIO_H
