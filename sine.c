#include <math.h>
#include <stdlib.h>
#include "unit_plugin.h"
#include "audio.h"

struct sine_data_s {
    double *l, *r;
    double *f, *a;
    int i;
};

typedef struct sine_data_s *sine_data_ptr;

static void* sine_note_new()
{
    sine_data_ptr p = (sine_data_ptr) malloc(sizeof(struct sine_data_s));
    p->i = 0;
    return p;
}

static void sine_note_free(void *ptr)
{
    free(ptr);
}

static void sine_connect_port(void *data, int port, double *d)
{
    sine_data_ptr p = (sine_data_ptr) data;

    switch (port) {
	case 0:
	    p->l = d;
	    break;
	case 1:
	    p->r = d;
	    break;
	case 2:
	    p->f = d;
	    break;
	case 3:
	    p->a = d;
	    break;
	default:
	    break;
    }
}

static void run(void *data)
{
    sine_data_ptr p = (sine_data_ptr) data;
    double d;

    d = sin(*p->f * p->i * 2.0 * M_PI / samprate) * 0.2;
    d *= *p->a;
    p->i = (p->i + 1) % samprate;

    *p->l = *p->r = d;
}

static void put_desc(unit_info_ptr ui, int i, char *id, int type)
{
    ui->port_desc[i].type = type;
    ui->port_desc[i].id = id;
}

void unit_info_init(unit_info_ptr ui)
{
    ui->id = "Sine Demo Unit";
    ui->name = "Sine";
    ui->port_count = 4;
    ui->port_desc = (port_desc_ptr) malloc(ui->port_count * sizeof(port_desc_t));
    put_desc(ui, 0, "out0", up_output);
    put_desc(ui, 1, "out1", up_output);
    put_desc(ui, 2, "freq", up_input);
    put_desc(ui, 3, "amp", up_input);
    ui->connect_port = sine_connect_port;
    ui->run = run;
    ui->note_new = sine_note_new;
    ui->note_free = sine_note_free;
}
