#ifndef TRACK_H
#define TRACK_H

#include "darray.h"

enum {
    ev_noteon = 0,
    ev_noteoff
};

struct event_s {
    int delta;
    int type;
    int x1, x2;
};
typedef struct event_s event_t[1];
typedef struct event_s *event_ptr;

struct track_s {
    char *id;
    darray_t event;
};
typedef struct track_s track_t[1];
typedef struct track_s *track_ptr;

void track_remove_all(track_ptr t);
void track_clear(track_ptr t);
void track_init(track_ptr t, char *id);
void track_add_event(track_ptr t, int delta, int type, int x1, int x2);
static inline int track_is_empty(track_ptr t)
{
    return !t->event->count;
}

#endif //TRACK_H
