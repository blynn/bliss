#ifndef SONG_AREA_H
#define SONG_AREA_H

#include "widget.h"
#include "song.h"
#include "grid.h"

struct song_area_s {
    grid_t grid;
    song_ptr song;
    int step;
};

typedef struct song_area_s *song_area_ptr;
typedef struct song_area_s song_area_t[1];

void song_area_init(song_area_ptr);
song_area_ptr song_area_new();
void song_area_edit(song_area_ptr, song_ptr song);

#endif //SONG_AREA_H
