#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "track.h"
#include "util.h"

void track_rewind(track_ptr t)
{
    t->play_pattern = NULL;
}

void track_init(track_ptr t, machine_ptr m)
{
    t->machine = m;
    t->first->next = NULL;
    track_rewind(t);
}

void track_clear(track_ptr t)
{
    //TODO: clear list
}

tcell_ptr tcell_new(char *text, int tick)
{
    tcell_ptr c = (tcell_ptr) malloc(sizeof(struct tcell_s));
    c->text = strclone(text);
    c->type = event_normal;
    c->tick = tick;
    c->next = NULL;
    return c;
}

void tcell_clear(tcell_ptr c)
{
    free(c->text);
}

int tcellcmp(tcell_ptr c1, tcell_ptr c2)
{
    return c1->tick - c2->tick;
}

tcell_ptr track_tcell_at(track_ptr t, int tick)
{
    tcell_ptr c;
    for(c = t->first->next; c; c = c->next) {
	if (c->tick == tick) return c;
    }
    return NULL;
}

char *track_at(track_ptr t, int tick)
{
    tcell_ptr c = track_tcell_at(t, tick);
    if (!c) return NULL;;
    return c->text;
}

void track_delete(track_ptr t, int tick)
{
    tcell_ptr c, cn;
    c = t->first;
    for(;;) {
	cn = c->next;
	if (!cn) break;
	if (tick == cn->tick) {
	    c->next = cn->next;
	    tcell_clear(cn);
	    free(cn);
	    break;
	}
	c = cn;
    }
}

void track_put(track_ptr t, char *text, int tick)
{
    int i;
    tcell_ptr c, cnew;

    cnew = tcell_new(text, tick);

    c = t->first;
    for (;;) {
	if (!c->next) {
	    c->next = cnew;
	    return;
	}
	i = tcellcmp(cnew, c->next);
	if (!i) {
	    cnew->next = c->next->next;
	    tcell_clear(c->next);
	    free(c->next);
	    c->next = cnew;
	    return;
	}
	if (i < 0) {
	    cnew->next = c->next;
	    c->next = cnew;
	    return;
	}
	c = c->next;
    }
}

track_ptr track_new(machine_ptr m)
{
    track_ptr t;
    t = (track_ptr) malloc(sizeof(struct track_s));
    track_init(t, m);
    return t;
}

void track_tick(track_ptr t, int tick)
{
    darray_ptr a;
    int i, n;
    char *id = track_at(t, tick);

    if (id) {
	a = t->machine->pattern;
	n = a->count;
	for (i=0; i<n; i++) {
	    pattern_ptr p = (pattern_ptr) a->item[i];
	    if (!strcmp(id, p->id)) {
		t->play_pattern = p;
		t->play_tick = tick;
		break;
	    }
	}
    }
    if (t->play_pattern) {
	pattern_tick(t->play_pattern, tick - t->play_tick);
    }
}
