#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "machine.h"
#include "song.h"
#include "util.h"

enum {
    resolution = 1024,
    type_saw = 0,
    type_sine,
};

struct abass2_data_s {
    int ttl;
    int type;
    double f;
    double x;

    double x1, x2, y1, y2;
    double a0, a1, a2, b1, b2;

    double cutoff;
    double recipq;
};
typedef struct abass2_data_s *abass2_data_ptr;

static int first = 1;
static double sintable[resolution];

static double lowpass(abass2_data_ptr p, double x)
{
    double y;

    y = x * p->a0 + p->x1 * p->a1 + p->x2 * p->a2
	+ p->y1 * p->b1 + p->y2 * p->b2;
    p->x2 = p->x1;
    p->x1 = x;
    p->y2 = p->y1;
    p->y1 = y;
    return y;
}

static void computetaps(abass2_data_ptr p)
{
    double b2, b1, bd;

    b2 = 1.0 / tan(M_PI * p->cutoff);
    b1 = p->recipq * b2;
    b2 = b2 * b2;
    bd = 1.0 / (b1 + b2 + 1.0);

    p->b1 = -(2.0 - 2.0 * b2) * bd;
    p->b2 = -(b2 - b1 + 1.0) * bd;
    p->a0 = bd;
    p->a1 = 2 * bd;
    p->a2 = bd;
}

static void put_cutoff(abass2_data_ptr p, int i)
{
    p->cutoff = ((double) (i + 1)) / 256;
}

static void put_resonance(abass2_data_ptr p, int i)
{
    p->recipq = 1.0 - ((double) i) / 256;
}

static void abass2_work(machine_t m, double *l, double *r)
{
    abass2_data_ptr p = (abass2_data_ptr) m->data;
    double x;
    int j;

    if (p->ttl) {
	x = p->x;
	switch (p->type) {
	    case type_sine:
		j = resolution * x / samprate;
		*l = sintable[j];
		break;
	    case type_saw:
		*l = 0.5 - x / samprate;
		break;
	    default:
		break;
	}
	x += p->f;
	if (x >= samprate) x -= samprate;
	p->x = x;
	//very simple decay
	if (p->ttl < 5000) {
	    *l *= (double) p->ttl / 5000;
	}
	p->ttl--;
	*l = lowpass(p, *l);
	*r = *l;
    }
}

static void abass2_static_init()
{
    int i;
    for (i=0; i<resolution; i++) {
	sintable[i] = 0.5 * sin(((double) i / (double) resolution) * M_PI * 2);
    }
}

static void abass2_init(machine_t m)
{
    abass2_data_ptr p;
    m->data = malloc(sizeof(struct abass2_data_s));
    p = (abass2_data_ptr) m->data;
    if (first) {
	first = 0;
	abass2_static_init();
    }
    p->ttl = 0;
    p->type = type_saw;
    p->x1 = p->x2 = 0;
    p->y1 = p->y2 = 0;
    put_cutoff(p, 16);
    put_resonance(p, 16);
    computetaps(p);
}

static void abass2_clear(machine_t m)
{
    free(m->data);
}

static void abass2_parse(machine_t m, char *cmd, int col)
{
    abass2_data_ptr p = (abass2_data_ptr) m->data;
    char *s;
    int n;

    s = cmd;
    switch (*s) {
	case 'n': //note
	    s++; if (!*s) break;
	    n = notechar_to_int(*s);
	    s++; if (!*s) break;
	    if (*s == '#') {
		n++;
		s++; if (!*s) break;
	    }
	    n = n + 12 * (*s - '0');
	    p->f = note_to_freq(n);
	    p->x = 0;
	    p->ttl = m->song->samptick;
	    break;
	case 'l': //length
	    s++; if (!*s) break;
	    n = hex_to_int(*s);
	    s++; if (!*s) break;
	    n = n * 16 + hex_to_int(*s);
	    p->ttl = n * m->song->samptick;
	    break;
	case 'c': //length
	    s++; if (!*s) break;
	    n = hex_to_int(*s);
	    s++; if (!*s) break;
	    n = n * 16 + hex_to_int(*s);
	    put_cutoff(p, n);
	    computetaps(p);
	    break;
	case 'r': //length
	    s++; if (!*s) break;
	    n = hex_to_int(*s);
	    s++; if (!*s) break;
	    n = n * 16 + hex_to_int(*s);
	    put_resonance(p, n);
	    computetaps(p);
	    break;
	default:
	    break;
    }
}

static void abass2_tick(machine_t m)
{
}

void machine_info_init(machine_info_ptr mi)
{
    mi->type = machine_generator;
    mi->id = "Alpha Bass 2";
    mi->name = "aBass2";
    mi->init = abass2_init;
    mi->clear = abass2_clear;
    mi->work = abass2_work;
    mi->parse = abass2_parse;
    mi->tick = abass2_tick;
}
