#include "bliss.h"
#include "track.h"

struct track_state_s {
    int time_last; //time of last event
    int index; //of next event in event list
};
typedef struct track_state_s *track_state_ptr;
typedef struct track_state_s track_state_t[1];

struct play_state_s {
    darray_t ts; //track state
    int time; //current time
    int delta; //time until next event (from any track)
    int finished;
};
typedef struct play_state_s *play_state_ptr;
typedef struct play_state_s play_state_t[1];

static play_state_t ps;

void play_state_init()
{
    darray_init(ps->ts);
}

int play_state_finished()
{
    return ps->finished;
}

int play_state_delta()
{
    return ps->delta;
}

void play_state_rewind()
{
    int mindelta = -1;
    int i, n;
    n = orch->insnode->count;
    darray_forall(ps->ts, free);
    darray_remove_all(ps->ts);
    ps->finished = 1;
    for (i=0; i<n; i++) {
	node_ptr node = orch->insnode->item[i];
	ins_ptr ins = ((node_data_ptr) node->data)->ins;
	track_ptr tr = ins->track;
	track_state_ptr ts = (track_state_ptr) malloc(sizeof(track_state_t));
	ts->index = 0;
	ts->time_last = 0;
	darray_append(ps->ts, ts);
	if (tr->event->count) {
	    event_ptr e = tr->event->item[0];
	    if ((mindelta == -1 || e->delta < mindelta)) {
		mindelta = e->delta;
	    }
	    ps->finished = 0;
	}
    }
    ps->time = 0;
    ps->delta = mindelta;
}

void play_state_advance(void (*handler)(ins_ptr, event_ptr))
{
    int i, n;
    int mindelta = -1;
    n = orch->insnode->count;
    ps->finished = 1;
    ps->time += ps->delta;
    for (i=0; i<n; i++) {
	node_ptr node = orch->insnode->item[i];
	ins_ptr ins = ((node_data_ptr) node->data)->ins;
	track_ptr tr = ins->track;
	track_state_ptr ts = ps->ts->item[i];
	int ei = ts->index;
	event_ptr e;

	if (ei == tr->event->count) continue;
	e = tr->event->item[ei];

	if (ps->time == e->delta + ts->time_last) {
	    ts->time_last = ps->time;
	    for (;;) {
		handler(ins, e);
		ei++;
		if (ei == tr->event->count) break;
		e = tr->event->item[ei];
		if (e->delta > 0) {
		    if (mindelta == -1 || e->delta < mindelta) {
			mindelta = e->delta;
		    }
		    ps->finished = 0;
		    break;
		}
	    }
	    ts->index = ei;
	} else {
	    int delta = e->delta - (ps->time - ts->time_last);
	    if (mindelta == -1 || delta < mindelta) {
		mindelta = delta;
	    }
	    ps->finished = 0;
	}
    }
    ps->delta = mindelta;
}

orch_t orch;
