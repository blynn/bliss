#include <stdlib.h>
#include <string.h>
#include "machine.h"
#include "song.h"
#include "util.h"

enum {
    track_max = 16,
};

struct atracker_data_s {
    double freq;
    int playing;
    int sample;
    double i;
};
typedef struct atracker_data_s *atracker_data_ptr;

static void atracker_work(machine_t m, double *l, double *r)
{
    atracker_data_ptr parray = (atracker_data_ptr) m->data;
    int i;
    double d;

    for (i=0; i<track_max; i++) {
	atracker_data_ptr p = &parray[i];
	if (p->playing) {
	    wave_ptr w = m->song->wave[p->sample];
	    d = sample_at(w->data, p->i);
	    d /= 1 << 15;
	    d *= 0.5 * w->volume;
	    p->i += p->freq * w->recipfreq;
	    if (p->i >= w->sample_count) p->playing = 0;
	    *l += d;
	    *r += d;
	}
    }
}

static void atracker_init(machine_t m)
{
    atracker_data_ptr p;
    int i;
    m->data = malloc(track_max * sizeof(struct atracker_data_s));

    for (i=0; i<track_max; i++) {
	p = &((atracker_data_ptr) m->data)[i];
	p->playing = 0;
	p->sample = 0;
    }
}

static void atracker_clear(machine_t m)
{
}

static void atracker_parse(machine_t m, cell_t c, int col)
{
    int i, j;
    atracker_data_ptr p;
    char *s;
    int n;

    if (col < 1) return;
    i = (col - 1) / 5;
    j = (col - 1) % 5;

    p = &((atracker_data_ptr) m->data)[i];

    s = c->data.s;
    if (!*s) return;
    switch(j) {
	case 0:
	    if (!strcmp(s, "off")) {
		p->playing = 0;
	    } else {
		n = notechar_to_int(*s);
		s++; if (!*s) return;
		if (*s == '#') {
		    n++;
		    s++; if (!*s) return;
		}
		n += 12 * (*s - '0');
		p->freq = note_to_freq(n);
		p->i = 0;
		p->playing = 1;
	    }
	    break;
	case 1:
	    n = hex_to_int(*s);
	    s++; if (!*s) break;
	    n = n * 16 + hex_to_int(*s);
	    p->sample = n;
	    break;
	default:
	    break;
    }
}

static void atracker_tick(machine_t m)
{
}

void machine_info_init(machine_info_ptr mi)
{
    mi->type = machine_generator;
    mi->id = "Alpha Tracker";
    mi->name = "aTracker";
    mi->init = atracker_init;
    mi->clear = atracker_clear;
    mi->work = atracker_work;
    mi->parse = atracker_parse;
    mi->tick = atracker_tick;
}
