#ifndef SONG_H
#define SONG_H

#include "machine.h"
#include "wave.h"

enum {
    wave_max = 256,
};

struct song_s {
    machine_ptr master;
    darray_t machine;
    darray_t edge;
    wave_ptr wave[wave_max];
    int bpm;
    int tpb;
    int samptick; //samples per tick
    int sampcount; //total samples computed
    int sampmod;
    int tickcount;
    int tickmod;
    int x, y; //width, height
};

typedef struct song_s *song_ptr;
typedef struct song_s song_t[1];

void song_init(song_ptr s);
void song_clear(song_ptr s);
edge_ptr song_create_edge(song_ptr s, machine_ptr src, machine_ptr dst);
void song_del_edge(song_ptr s, edge_ptr e);
void song_add_machine(song_ptr, machine_ptr m);
void song_del_machine(song_ptr s, machine_ptr m);
int song_is_connected(song_ptr s, machine_ptr src, machine_ptr dst);
void song_next_sample(song_ptr s, double *l, double *r);
void song_rewind(song_ptr s);
int song_load(song_ptr s, char *filename);
int song_save(song_ptr s, char *filename);
machine_ptr song_create_machine_auto_id(song_ptr s, char *file);
machine_ptr song_create_machine(song_ptr s, char *gearid, char *id);
void song_put_bpm_tpb(song_ptr, int, int);
void song_put_wave(song_ptr s, wave_ptr w, int i);
#endif //SONG_H
