#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "voice.h"

#include "util.h"

static void voice_note_free(voice_t voice, note_ptr note)
//doesn't update note_list
{
    int i;
    darray_ptr l = voice->graph->node_list;
    int gencount = l->count;
    darray_remove(voice->note_list, note);
    for (i=0; i<gencount; i++) {
	gen_ptr g = ((node_data_ptr) ((node_ptr) l->item[i])->data)->gen;
	gen_note_free(g, note->gen_data[i]->data);
	free(note->gen_data[i]);
    }
    free(note->gen_data);
    voice->keyboard[note->noteno] = NULL;
    free(note);
}

note_ptr voice_note_on(voice_t voice, int noteno, double volume)
{
    int i;
    darray_ptr l = voice->graph->node_list;
    int gencount = l->count;
    double freq = note_to_freq(noteno);
    note_ptr res;

    if ((res = voice->keyboard[noteno])) {
	voice_note_free(voice, res);
    }
    res = voice->keyboard[noteno] = malloc(sizeof(note_t));
    res->noteno = noteno;
    res->freq = freq;
    res->volume = volume;

    res->is_off = 0;

    res->voice = voice;

    res->gen_data = malloc(sizeof(gen_data_ptr) * gencount);

    for (i=0; i<gencount; i++) {
	gen_ptr g = ((node_data_ptr) ((node_ptr) l->item[i])->data)->gen;
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
    note_ptr note = voice->keyboard[noteno];
    if (note) {
	note->is_off = -1;
    }
}

static void per_node_recurse_tick(voice_t voice, node_ptr node, note_ptr note)
{
    int i;
    node_data_ptr p = node->data;
    gen_data_ptr pnote = note->gen_data[p->gen_index];
    double invalue[p->gen->info->port_count];

    //zero values at input ports
    memset(invalue, 0, sizeof(double) * p->gen->info->port_count);

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
	    per_node_recurse_tick(voice, n1, note);
	}

	portno = *((int *) e->data);
	invalue[portno] += p1->output;
    }

    //compute output value
    p->output = gen_tick(p->gen, pnote, invalue);
    if (pnote->alive) note->alive = 1;
}

static void recurse_tick(voice_t voice, node_ptr node, note_ptr note)
{
    int i;
    node_data_ptr p = node->data;
    gen_data_ptr gdp = note->gen_data[p->gen_index];
    double invalue[p->gen->info->port_count];

    //zero values at input ports
    memset(invalue, 0, sizeof(double) * p->gen->info->port_count);

    //compute input values recursively
    p->visited = -1;

    for (i=0; i<node->in->count; i++) {
	edge_ptr e = node->in->item[i];
	node_ptr n1 = e->src;
	node_data_ptr p1 = (node_data_ptr) n1->data;
	int portno = *((int *) e->data);

	//the `freq' node is special: it always
	//outputs the frequency of the note
	if (n1 == voice->freq) {
	    invalue[portno] += note->freq;
	//otherwise compute node's output if we haven't already
	} else {
	    if (!p1->visited) recurse_tick(voice, n1, note);
	    invalue[portno] += note->gen_data[p1->gen_index]->output;
	}
    }

    //compute output value
    gdp->output = gen_tick(p->gen, gdp, invalue);
    if (gdp->alive) note->alive = 1;
}

double voice_tick(voice_t voice)
{
    double res = 0.0;
    int i = 0;
    node_ptr outnode = voice->out;
    node_data_ptr outp = outnode->data;

    while (i<voice->note_list->count) {
	note_ptr note = voice->note_list->item[i];

	if (note->is_off && !note->alive) {
	    voice_note_free(voice, note);
	} else {
	    void clear_visited(node_ptr node) {
		((node_data_ptr) node->data)->visited = 0;
	    }

	    graph_forall_node(voice->graph, clear_visited);
	    ((node_data_ptr) voice->freq->data)->visited = 1;

	    note->alive = 0;
	    if (0) {
		//one output per node version
		per_node_recurse_tick(voice, outnode, note);
		res += outp->output * note->volume;
	    } else {
		recurse_tick(voice, outnode, note);
		res += note->gen_data[outp->gen_index]->output * note->volume;
	    }
	    i++;
	}
    }
    return res;
}

node_ptr node_from_gen_info(graph_ptr graph, gen_info_t gi, char *id)
{
    gen_ptr g;
    node_data_ptr p;

    g = gen_new(gi);
    p = malloc(sizeof(node_data_t));
    if (!strncmp("funk", gi->id, 4)) {
	p->type = node_type_funk;
    } else {
	p->type = node_type_normal;
    }
    p->id = strclone(id);
    p->gen = g;
    //TODO: hackish:
    p->gen_index = graph->node_list->count;
    p->output = 0.0;
    return graph_add_node(graph, p);
}

node_ptr voice_add_gen(voice_t voice, gen_info_t gi, char *id)
{
    return node_from_gen_info(voice->graph, gi, id);
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

static void del_node_cb(node_ptr node, void *data)
{
    voice_ptr voice = data;
    node_data_ptr p = node->data;

    //TODO: lock audio
    voice_remove_all_notes(voice);
    if (p->type == node_type_funk || p->type == node_type_normal) {
	gen_free(p->gen);
    }
    free(p);
}

void voice_init(voice_t voice, char *s)
{
    int i;
    for (i=0; i<128; i++) voice->keyboard[i] = NULL;
    voice->notemin = 0;
    voice->notemax = 127;
    graph_init(voice->graph);
    graph_put_delete_node_cb(voice->graph, del_node_cb, voice);
    darray_init(voice->note_list);
    voice->id = strclone(s);

    voice->out = voice_add_gen(voice, &out_info, "out");
    voice->freq = voice_add_gen(voice, &dummy_info, "freq");
}

voice_ptr voice_new(char *s)
{
    voice_ptr res;
    res = (voice_ptr) malloc(sizeof(voice_t));
    voice_init(res, s);
    return res;
}

void voice_clear(voice_ptr voice)
{
    free(voice->id);
    voice_remove_all_notes(voice);

    graph_clear(voice->graph);
}

void voice_free(voice_ptr voice)
{
    voice_clear(voice);
    free(voice);
}

edge_ptr voice_connect(voice_ptr voice, node_ptr src, node_ptr dst, int dstport)
{
    int *ip = malloc(sizeof(int));
    *ip = dstport;
    return graph_add_edge(voice->graph, src, dst, ip);
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
