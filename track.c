#include "track.h"
#include "util.h"

void track_remove_all(track_ptr t)
{
    darray_forall(t->event, free);
    darray_remove_all(t->event);
}

void track_clear(track_ptr t)
{
    track_remove_all(t);
    free(t->id);
    darray_clear(t->event);
}

void track_init(track_ptr t, char *id)
{
    t->id = strclone(id);
    darray_init(t->event);
}

void track_add_event(track_ptr t, int delta, int type, int x1, int x2)
{
    event_ptr e;

    e = (event_ptr) malloc(sizeof(event_t));
    e->delta = delta;
    e->type = type;
    e->x1 = x1;
    e->x2 = x2;
    darray_append(t->event, e);
}
