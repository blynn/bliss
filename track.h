#ifndef TRACK_H
#define TRACK_H

#include "machine.h"

enum {
    event_normal = 0,
    event_mute,
    event_break,
    event_thru,
};

struct tcell_s {
    int tick;
    int type;
    char *text;
    struct tcell_s *next;
};

typedef struct tcell_s *tcell_ptr;
typedef struct tcell_s tcell_t[1];

struct track_s {
    struct machine_s *machine;
    tcell_t first;
    pattern_ptr play_pattern;
    int play_tick;
};

typedef struct track_s *track_ptr;
typedef struct track_s track_t[1];

void track_init(track_ptr t, struct machine_s *m);
void track_clear(track_ptr t);
track_ptr track_new(struct machine_s *m);

void track_put(track_ptr t, char *text, int tick);
char *track_at(track_ptr t, int tick);
void track_delete(track_ptr t, int tick);
void track_tick(track_ptr t, int tick);
void track_rewind(track_ptr t);
#endif //TRACK_H
