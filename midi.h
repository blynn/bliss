#ifndef MIDI_H
#define MIDI_H

struct midi_cb_s {
    void (*note_on)(int noteno, int vel);
    void (*note_off)(int noteno);
};

typedef struct midi_cb_s midi_cb_t[1];
typedef struct midi_cb_s *midi_cb_ptr;

int midi_start(midi_cb_ptr p);
void midi_stop();

#endif //MIDI_H
