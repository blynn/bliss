#include <stdio.h>
#include <stdlib.h>
#include "pattern.h"
#include "machine.h"
#include "util.h"
#include "cell.h"

struct celllist_s {
    int x;
    cell_t cell;
    struct celllist_s *prev;
    struct celllist_s *next;
};

typedef struct celllist_s *celllist_ptr;
typedef struct celllist_s celllist_t[1];

struct rowlist_s {
    int y;
    celllist_t first;
    struct rowlist_s *prev;
    struct rowlist_s *next;
};

typedef struct rowlist_s *rowlist_ptr;
typedef struct rowlist_s rowlist_t[1];

void pattern_init(pattern_ptr p, machine_ptr m)
{
    p->id = NULL;
    p->machine = m;
    p->first = (rowlist_ptr) malloc(sizeof(rowlist_t));
    p->first->next = NULL;
    p->first->prev = NULL;
    p->last = p->first;
    darray_append(m->pattern, p);
}

pattern_ptr pattern_new(machine_ptr m)
{
    pattern_ptr p;
    p = (pattern_ptr) malloc(sizeof(struct pattern_s));
    pattern_init(p, m);
    return p;
}

static rowlist_ptr row_at(pattern_ptr p, int y)
{
    rowlist_ptr rp;

    for (rp=p->first->next; rp; rp=rp->next) {
	if (y == rp->y) {
	    return rp;
	}
    }
    return NULL;
}

static void play_row(pattern_ptr p, rowlist_ptr rp)
{
    celllist_ptr c;

    for (c=rp->first->next; c; c=c->next) {
	machine_parse(p->machine, c->cell, c->x);
    }

    machine_tick(p->machine);
}

void pattern_tick(pattern_ptr p, int tick)
{
    rowlist_ptr rp;

    rp = row_at(p, tick);

    if (rp) play_row(p, rp);
}

cell_ptr pattern_cell_at(pattern_ptr p, int x, int y)
{
    rowlist_ptr rp;
    celllist_ptr c;

    rp = row_at(p, y);

    if (!rp) return NULL;

    for (c=rp->first->next; c; c=c->next) {
	if (x == c->x) return c->cell;
    }
    return NULL;
}

char *pattern_at(pattern_ptr p, int x, int y)
{
    cell_ptr c = pattern_cell_at(p, x, y);
    if (c) return cell_to_text(c);
    return NULL;
}

void celllist_clear(celllist_ptr c)
{
    cell_clear(c->cell);
}

static void row_delete(rowlist_ptr rp)
{
    celllist_ptr c, cn;

    c = rp->first->next;
    for (;;) {
	if (!c) break;
	cn = c->next;

	celllist_clear(c);
	free(c);

	c = cn;
    }
}

void pattern_clear(pattern_ptr p)
{
    rowlist_ptr rp, rpn;

    rp = p->first->next;
    for (;;) {
	if (!rp) break;
	rpn = rp->next;

	row_delete(rp);
	free(rp);

	rp = rpn;
    }

    free(p->id);
    free(p->first);
}

static rowlist_ptr insert_new_row(pattern_ptr p, int y)
{
    rowlist_ptr rp, newr;
    for (rp = p->first; rp->next && rp->next->y < y; rp = rp->next);
    newr = (rowlist_ptr) malloc(sizeof(rowlist_t));
    newr->first->next = NULL;
    newr->first->prev = NULL;
    newr->y = y;
    newr->prev = rp;
    newr->next = rp->next;
    rp->next = newr;
    if (newr->next) newr->next->prev = newr;
    else p->last = newr;
    return newr;
}

static celllist_ptr col_at(rowlist_ptr rp, int x)
{
    celllist_ptr c;

    for (c=rp->first->next; c; c=c->next) {
	if (x == c->x) {
	    return c;
	}
    }
    return NULL;
}

static void insert_col(rowlist_ptr rp, celllist_ptr newc)
{
    celllist_ptr c;
    for (c = rp->first; c->next && c->next->x < newc->x; c = c->next);
    newc->prev = c;
    newc->next = c->next;
    c->next = newc;
    if (newc->next) newc->next->prev = newc;
}

static celllist_ptr insert_new_col(rowlist_ptr rp, int x)
{
    celllist_ptr newc;
    newc = (celllist_ptr) malloc(sizeof(celllist_t));
    newc->x = x;
    insert_col(rp, newc);
    return newc;
}

void pattern_put(pattern_ptr p, char *text, int x, int y)
{
    rowlist_ptr rp;
    celllist_ptr c;

    rp = row_at(p, y);
    if (!rp) rp = insert_new_row(p, y);

    c = col_at(rp, x);
    if (!c) c = insert_new_col(rp, x);
    else cell_clear(c->cell);

    machine_cell_init(c->cell, p->machine, text, x);
}

static int row_is_empty(rowlist_ptr rp)
{
    return !rp->first->next;
}

static void delete_col(pattern_ptr p, rowlist_ptr rp, celllist_ptr c)
    //removes c from rowlist, does not delete contents of c
{
    celllist_ptr c1 = c->next;
    c->prev->next = c1;
    if (c1) c1->prev = c->prev;

    //delete possibly empty row
    if (row_is_empty(rp)) {
	rowlist_ptr rp1;
	rp1 = rp->prev;
	rp1->next = rp->next;
	if (rp->next) {
	    rp->next->prev = rp1;
	} else p->last = rp1;
	free(rp);
    }
}

void pattern_insert(pattern_ptr p, int x, int y)
{
    rowlist_ptr rp, rpp, rp1;
    celllist_ptr c;

    if (!p->last->prev) return; //empty pattern

    for (rp=p->last; rp != p->first && rp->y >= y;) {
	rpp = rp->prev; //can't put this in for loop
	    //because rp may get removed
	c = col_at(rp, x);
	if (c) {
	    rp1 = row_at(p, rp->y + 1);
	    if (!rp1) rp1 = insert_new_row(p, rp->y + 1);
	    delete_col(p, rp, c);
	    insert_col(rp1, c);
	}
	rp = rpp;
    }
}

void pattern_delete(pattern_ptr p, int x, int y)
{
    rowlist_ptr rp, rpn, rp1;
    celllist_ptr c;

    pattern_remove(p, x, y);

    //jump to first row >= y
    for (rp=p->first->next; rp && rp->y < y; rp = rp->next);
    if (!rp) return;

    for (;;) {
	rpn = rp->next;
	c = col_at(rp, x);
	if (c) {
	    rp1 = row_at(p, rp->y - 1);
	    if (!rp1) rp1 = insert_new_row(p, rp->y - 1);
	    delete_col(p, rp, c);
	    insert_col(rp1, c);
	}
	if (!rpn) break;
	rp = rpn;
    }
}

void pattern_remove(pattern_ptr p, int x, int y)
{
    celllist_ptr c;
    rowlist_ptr rp;

    rp = row_at(p, y);
    if (!rp) return;
    c = col_at(rp, x);
    if (!c) return;

    delete_col(p, rp, c);

    celllist_clear(c);
    free(c);
}

void pattern_print(pattern_ptr p, FILE *fp)
{
    rowlist_ptr rp;
    celllist_ptr c;

    fprintf(fp, "\t\tid %s\n", p->id);
    for (rp=p->first->next; rp; rp=rp->next) {
	for (c=rp->first->next; c; c=c->next) {
	    fprintf(fp, "\t\tcell %d %d %s\n", c->x, rp->y, cell_to_text(c->cell));
	}
    }
}
