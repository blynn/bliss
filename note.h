#ifndef NOTE_H
#define NOTE_H

#include "gen.h"

struct voice_s;

struct note_s {
    double freq;
    double volume;
    struct voice_s *voice;
    int is_off;
    int alive;
    gen_data_ptr *gen_data;
};

typedef struct note_s note_t[1];
typedef struct note_s *note_ptr;

double note_to_freq(int n);

#endif //NOTE_H
