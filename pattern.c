#include <stdio.h>
#include <stdlib.h>
#include "pattern.h"
#include "machine.h"
#include "util.h"

void pattern_init(pattern_ptr p, machine_ptr m)
{
    p->id = NULL;
    p->machine = m;
    p->first->next = NULL;
    darray_append(m->pattern, p);
}

pattern_ptr pattern_new(machine_ptr m)
{
    pattern_ptr p;
    p = (pattern_ptr) malloc(sizeof(struct pattern_s));
    pattern_init(p, m);
    return p;
}

static void play_row_at(pattern_ptr p, cell_ptr start)
{
    cell_ptr c = start;
    do {
	machine_parse(p->machine, c->text, c->x);
	c = c->next;
    } while(c && c->y == start->y);
    machine_tick(p->machine);
}

void pattern_tick(pattern_ptr p, int tick)
{
    cell_ptr c;

    for (c=p->first->next; c; c=c->next) {
	if (c->y > tick) return;
	if (c->y == tick) {
	    play_row_at(p, c);
	}
    }
}

cell_ptr pattern_cell_at(pattern_ptr p, int x, int y)
{
    cell_ptr c;

    for (c=p->first->next; c; c=c->next) {
	if (y < c->y) return NULL;
	if (y == c->y && x == c->x) return c;
    }
    return NULL;
}

char *pattern_at(pattern_ptr p, int x, int y)
{
    cell_ptr c;

    for (c=p->first->next; c; c=c->next) {
	if (y < c->y) return NULL;
	if (y == c->y && x == c->x) return c->text;
    }
    return NULL;
}

void cell_init(cell_ptr c, char *text, int x, int y)
{
    c->text = strclone(text);
    c->x = x;
    c->y = y;
    c->next = NULL;
}

void cell_clear(cell_ptr c)
{
    free(c->text);
}

cell_ptr cell_new(char *text, int x, int y)
{
    cell_ptr c = (cell_ptr) malloc(sizeof(struct cell_s));
    cell_init(c, text, x, y);
    return c;
}

void pattern_clear(pattern_ptr p)
{
    cell_ptr c, cn;

    c = p->first->next;
    for (;;) {
	if (!c) break;
	cn = c->next;

	cell_clear(c);
	free(c);

	c = cn;
    }

    free(p->id);
}

static int cellcmp(cell_ptr c1, cell_ptr c2)
{
    int r;

    r = c1->y - c2->y;
    if (!r) r = c1->x - c2->x;
    return r;
}

void pattern_put(pattern_ptr p, char *text, int x, int y)
{
    cell_ptr cnew;
    cell_ptr c;
    int i;

    cnew = cell_new(text, x, y);

    c = p->first;
    for (;;) {
	if (!c->next) {
	    c->next = cnew;
	    return;
	}
	i = cellcmp(cnew, c->next);

	if (!i) {
	    cnew->next = c->next->next;
	    cell_clear(c->next);
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

void pattern_delete(pattern_ptr p, int x, int y)
{
    cell_ptr c, cn;

    c = p->first;
    for (;;) {
	cn = c->next;
	if (!cn) break;
	if (y == cn->y && x == cn->x) {
	    c->next = cn->next;
	    cell_clear(cn);
	    free(cn);
	}
	c = cn;
    }
}
