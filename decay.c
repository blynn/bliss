#include <stdlib.h>
#include "unit_plugin.h"

struct decay_data_s {
    int i;
    double *out;
};

typedef struct decay_data_s *decay_data_ptr;

static void* decay_note_new()
{
    decay_data_ptr p = (decay_data_ptr) malloc(sizeof(struct decay_data_s));
    p->i = 0;
    return p;
}

static void decay_note_free(void *ptr)
{
    free(ptr);
}

static void decay_connect_port(void *data, int port, double *d)
{
    decay_data_ptr p = (decay_data_ptr) data;

    switch (port) {
	case 0:
	    p->out = d;
	    break;
	default:
	    break;
    }
}

static void run(void *data)
{
    decay_data_ptr p = (decay_data_ptr) data;
    double d;

    d = (20000 - p->i) / 20000.0;
    p->i++;

    *p->out = d;
}

static void put_desc(unit_info_ptr ui, int i, char *id, int type)
{
    ui->port_desc[i].type = type;
    ui->port_desc[i].id = id;
}

void unit_info_init(unit_info_ptr ui)
{
    ui->id = "Decay Demo Unit";
    ui->name = "Decay";
    ui->port_count = 1;
    ui->port_desc = (port_desc_ptr) calloc(ui->port_count, sizeof(port_desc_t));
    put_desc(ui, 0, "out", up_output);
    ui->connect_port = decay_connect_port;
    ui->run = run;
    ui->note_new = decay_note_new;
    ui->note_free = decay_note_free;
}
