#ifndef WAVE_H
#define WAVE_H

struct wave_s {
    char *id;
    double volume;
    int sample_count;
    int data_length;
    unsigned char *data;
    int root_note;
    double freq, recipfreq;
};

typedef struct wave_s *wave_ptr;
typedef struct wave_s wave_t[1];

void wave_init(wave_ptr);
void wave_clear(wave_ptr);
wave_ptr wave_new();
int wave_sample_at(wave_ptr w, int i);
void wave_put_root_note(wave_ptr w, int n);

#endif //WAVE_H
