#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "voice.h"

static inline char *strclone(char *s)
{
    char *res = malloc(strlen(s) + 1);
    strcpy(res, s);
    return res;
}

note_ptr voice_note_on(voice_t voice, int noteno, double volume)
{
    int i;
    int gencount = voice->node_list->count;
    double freq = note_to_freq(noteno);
    note_ptr res;

    if (voice->keyboard[noteno]) {
	//TODO: could kill the note off instead?
	voice_note_off(voice, noteno);
    }
    voice->keyboard[noteno] = res = malloc(sizeof(note_t));
    res->freq = freq;
    res->volume = volume;

    res->is_off = 0;

    res->voice = voice;

    res->gen_data = malloc(sizeof(gen_data_ptr) * gencount);

    for (i=0; i<gencount; i++) {
	gen_ptr g = ((node_data_ptr) ((node_ptr) voice->node_list->item[i])->data)->gen;
	res->gen_data[i] = malloc(sizeof(gen_data_t));
	res->gen_data[i]->data = gen_note_on(g);
	res->gen_data[i]->alive = 0;
	res->gen_data[i]->note = res;
    }
    darray_append(voice->note_list, res);

    return res;
}

void voice_note_off(voice_ptr voice, int noteno)
{
    note_ptr n = voice->keyboard[noteno];
    if (n) {
	n->is_off = -1;
	voice->keyboard[noteno] = NULL;
    }
}

void voice_note_free(voice_t voice, note_ptr note)
{
    int i;
    int gencount = voice->node_list->count;
    for (i=0; i<gencount; i++) {
	gen_ptr g = ((node_data_ptr) ((node_ptr) voice->node_list->item[i])->data)->gen;
	gen_note_free(g, note->gen_data[i]->data);
	free(note->gen_data[i]);
    }
    free(note->gen_data);
    free(note);
}

static void recurse_tick(voice_t voice, node_ptr node, note_ptr note)
{
    int i;
    node_data_ptr p;
    gen_data_ptr pnote;
    double invalue[voice->maxport + 1];

    //zero values at input and output ports
    p = (node_data_ptr) node->data;
    pnote = note->gen_data[p->gen_index];
    for (i=0; i<p->gen->info->port_count; i++) {
	invalue[i] = 0.0;
    }

    p->output = 0.0;

    //compute input values recursively
    p->visited = -1;

    for (i=0; i<node->in->count; i++) {
	edge_ptr e = node->in->item[i];
	node_ptr n1 = e->src;
	node_data_ptr p1 = (node_data_ptr) n1->data;
	int portno;

	//the `freq' node is special: it always
	//outputs the frequency of the note
	if (n1 == voice->freq) {
	    p1->output = note->freq;
	//otherwise compute node's output if we haven't already
	} else if (!p1->visited) {
	    recurse_tick(voice, n1, note);
	}

	portno = *((int *) e->data);
	invalue[portno] += p1->output;
    }

    //compute output value
    p->output += gen_tick(p->gen, pnote, invalue);
    if (pnote->alive) note->alive = 1;
}

double voice_tick(voice_t voice)
{
    double res = 0.0;
    int i = 0;

    while (i<voice->note_list->count) {
	note_ptr note = voice->note_list->item[i];

	if (note->is_off && !note->alive) {
	    darray_remove_index(voice->note_list, i);
	    voice_note_free(voice, note);
	} else {
	    void clear_visited(void *data) {
		node_ptr node = data;
		((node_data_ptr) node->data)->visited = 0;
	    }
	    darray_forall(voice->node_list, clear_visited);
	    ((node_data_ptr) voice->freq->data)->visited = 1;

	    note->alive = 0;
	    recurse_tick(voice, voice->out, note);
	    res += ((node_data_ptr) voice->out->data)->output * note->volume;
	    i++;
	}
    }
    return res;
}

node_ptr voice_add_gen(voice_t voice, gen_info_t gi, char *id)
{
    node_ptr node;
    gen_ptr g;
    node_data_ptr p;

    node = node_new();
    g = gen_new(gi);
    node->data = p = malloc(sizeof(node_data_t));
    if (!strncmp("funk", gi->id, 4)) {
	p->type = node_type_funk;
    } else {
	p->type = node_type_normal;
    }
    p->id = strclone(id);
    p->gen = g;
    p->gen_index = voice->node_list->count;
    p->output = 0.0;

    darray_append(voice->node_list, node);

    if (gi->port_count > voice->maxport) {
	voice->maxport = g->info->port_count;
    }

    return node;
}

extern struct gen_info_s out_info;
extern struct gen_info_s dummy_info;

void voice_remove_all_notes(voice_t voice)
{
    void remove_note(void *data) {
	voice_note_free(voice, (note_ptr) data);
    }
    darray_forall(voice->note_list, remove_note);
    darray_remove_all(voice->note_list);
}

void voice_init(voice_t voice, char *s)
{
    int i;
    for (i=0; i<128; i++) voice->keyboard[i] = NULL;
    voice->notemin = 0;
    voice->notemax = 127;
    darray_init(voice->node_list);
    darray_init(voice->note_list);
    voice->id = strclone(s);

    voice->out = voice_add_gen(voice, &out_info, "out");
    voice->freq = voice_add_gen(voice, &dummy_info, "freq");
    voice->maxport = 0;
}

voice_ptr voice_new(char *s)
{
    voice_ptr res;
    res = malloc(sizeof(voice_t));
    voice_init(res, s);
    return res;
}

void voice_clear(voice_ptr voice)
{
    int i, j;

    free(voice->id);
    voice_remove_all_notes(voice);
    darray_clear(voice->note_list);

    //delete edges then nodes
    for (i=0; i<voice->node_list->count; i++) {
	node_ptr node = (node_ptr) voice->node_list->item[i];
	for (j=0; j<node->in->count; j++) {
	    edge_ptr e = (edge_ptr) node->in->item[j];
	    free(e->data);
	    free(e);
	}
    }
    for (i=0; i<voice->node_list->count; i++) {
	node_ptr node = (node_ptr) voice->node_list->item[i];
	node_data_ptr p = (node_data_ptr) node->data;
	free(p->id);
	gen_clear(p->gen);
	free(p->gen);
	node_clear(node);
	free(node->data);
	free(node);
    }
    darray_clear(voice->node_list);
}

void voice_free(voice_ptr voice)
{
    voice_clear(voice);
    free(voice);
}

edge_ptr voice_connect(voice_t voice, node_ptr src, node_ptr dst, int dstport)
{
    edge_ptr e;

    e = edge_new(src, dst);
    e->data = malloc(sizeof(int));
    *((int *) e->data) = dstport;
    return e;
}

void voice_disconnect(voice_t voice, edge_ptr e)
{
    free(e->data);
    edge_delete(e);
}

void set_param(node_ptr node, int n, double val)
{
    gen_ptr g = ((node_data_ptr) node->data)->gen;

    g->param[n] = val;
    g->info->param[n]->callback(g, val);
}

int no_of_param(node_ptr node, char *id)
{
    int i;
    gen_ptr g = ((node_data_ptr) node->data)->gen;
    param_ptr *p = g->info->param;
    for (i=0; i<g->info->param_count; i++) {
	if (!strcmp(p[i]->id, id)) {
	    return i;
	}
    }
    return -1;
}

void set_param_by_id(node_ptr node, char *id, double val)
{
    int i;
    i = no_of_param(node, id);
    set_param(node, i, val);
}

void funk_clear_program(gen_ptr g);
void funk_init_program(gen_ptr g, char *s);
char *funk_get_program(gen_ptr g);

int node_type(node_ptr node)
{
    node_data_ptr inp;

    inp = (node_data_ptr) node->data;
    return inp->type;
}

char *get_funk_program(node_ptr node)
{
    gen_ptr g;
    node_data_ptr inp;

    inp = (node_data_ptr) node->data;
    g = inp->gen;
    return funk_get_program(g);
}

void set_funk_program(node_ptr node, char *s)
{
    gen_ptr g;
    node_data_ptr inp;

    inp = (node_data_ptr) node->data;
    g = inp->gen;
    funk_clear_program(g);
    funk_init_program(g, s);
}
