#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ins.h"

static char *strclone(char *s)
{
    char *res = malloc(strlen(s) + 1);
    strcpy(res, s);
    return res;
}

note_ptr ins_note_on(ins_t ins, int noteno, double volume)
{
    int i;
    int gencount = ins->node_list->count;
    double freq = note_to_freq(noteno);
    note_ptr res;

    res = malloc(sizeof(note_t));
    res->freq = freq;
    res->volume = volume;

    res->is_off = 0;

    res->ins = ins;

    res->ref_count = ins->ref_count;

    res->gen_data = malloc(sizeof(note_data_ptr) * gencount);

    for (i=0; i<gencount; i++) {
	gen_ptr g = ((ins_node_ptr) ((node_ptr) ins->node_list->item[i])->data)->gen;
	res->gen_data[i] = malloc(sizeof(note_data_t));
	res->gen_data[i]->g = g;
	res->gen_data[i]->data = gen_note_on(g);
    }
    darray_append(ins->note_list, res);

    return res;
}

void ins_note_off(note_ptr n)
{
    n->is_off = -1;
    n->ref_count--;
}

void ins_note_free(ins_t ins, note_ptr note)
{
    int i;
    int gencount = ins->node_list->count;
    for (i=0; i<gencount; i++) {
	gen_ptr g = ((ins_node_ptr) ((node_ptr) ins->node_list->item[i])->data)->gen;
	gen_note_free(g, note->gen_data[i]->data);
	free(note->gen_data[i]);
    }
    free(note->gen_data);
    free(note);
}

/* Slower, unfortunately:
static void recurse_tick(ins_t ins, node_ptr node)
{
    int i;
    ins_node_ptr p = (ins_node_ptr) node->data;
    gen_ptr g = p->gen; 

    //compute input values recursively
    p->visited = -1;

    for (i=0; i<node->in->count; i++) {
	edge_ptr e = node->in->item[i];
	node_ptr n1 = e->src;
	ins_node_ptr p1 = (ins_node_ptr) n1->data;
	if (!p1->visited) {
	    recurse_tick(ins, n1);
	}
    }

    //compute output value
    p->output = 0.0;
    for (i=0; i<ins->note_list->count;) {
	int j, k;
	note_ptr note = ins->note_list->item[i];
	note_data_ptr pnote;
	double value[16]; //TODO: malloc this instead
	for (k=0; k<16; k++) value[k] = 0.0;

	int gindex;

	for (k=0;note->gen_data[k]->g != g;k++);
	gindex = k;
	pnote = note->gen_data[gindex];

	//for all nodes n coming into this one
	// find gen_index of n
	// value[n] += output[n]
	pnote->output = 0.0;
	for (j=0; j<node->in->count; j++) {
	    edge_ptr e = node->in->item[j];
	    node_ptr n1 = e->src;
	    gen_ptr g1 = ((ins_node_ptr) n1->data)->gen;
	    note_data_ptr pn1;

	    for (k=0;note->gen_data[k]->g != g1;k++);
	    pn1 = note->gen_data[k];

	    if (n1 == ins->freq) {
		value[*((int *) e->data)] += note->freq;
	    } else {
		value[*((int *) e->data)] += pn1->output;
	    }
	}

	//compute output of this note at this node
	if (!note->ref_count) {
	    darray_remove_index(ins->note_list, i);
	    ins_note_free(ins, note);
	} else {
	    if (note->is_off && g->note_off_tick) {
		int dead;
		pnote->output += gen_note_off_tick(g, pnote->data, value, &dead);
		if (dead) note->ref_count--;
	    } else {
		pnote->output += gen_tick(g, pnote->data, value);
	    }
	    i++;
	}
	p->output += pnote->output;
    }
}

double ins_tick(ins_t ins)
{
    int j;
    double res = 0;

    for (j=0; j<ins->node_list->count; j++) {
	node_ptr node = ins->node_list->item[j];
	((ins_node_ptr) node->data)->visited = 0;
    }
    ((ins_node_ptr) ins->freq->data)->visited = 1;

    recurse_tick(ins, ins->out);
    res = ((ins_node_ptr) ins->out->data)->output;

    return res;
}
*/
static void recurse_tick(ins_t ins, node_ptr node, note_ptr note)
{
    int i;
    ins_node_ptr p;
    note_data_ptr pnote;
    double invalue[ins->maxport];

    //zero values at input and output ports
    p = (ins_node_ptr) node->data;
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
	ins_node_ptr p1 = (ins_node_ptr) n1->data;
	int portno;

	//the `freq' node is special: it always
	//outputs the frequency of the note
	if (n1 == ins->freq) {
	    p1->output = note->freq;
	//otherwise compute node's output if we haven't already
	} else if (!p1->visited) {
	    recurse_tick(ins, n1, note);
	}

	portno = *((int *) e->data);
	invalue[portno] += p1->output;
    }

    //compute output value
    /*
    p->output += gen_tick(p->gen, pnote->data, pnote->value);
    */
    if (note->is_off && p->gen->note_off_tick) {
	int dead = 0;
	p->output += gen_note_off_tick(p->gen, pnote->data, invalue, &dead);
	if (dead) {
	    note->ref_count--;
	}
    } else {
	p->output += gen_tick(p->gen, pnote->data, invalue);
    }
}

double ins_tick(ins_t ins)
{
    int i, j;
    double res = 0;

    for (i=0; i<ins->note_list->count;) {
	note_ptr note = ins->note_list->item[i];

	if (!note->ref_count) {
	    darray_remove_index(ins->note_list, i);
	    ins_note_free(ins, note);
	} else {
	    for (j=0; j<ins->node_list->count; j++) {
		node_ptr node = ins->node_list->item[j];
		((ins_node_ptr) node->data)->visited = 0;
	    }
	    ((ins_node_ptr) ins->freq->data)->visited = 1;

	    recurse_tick(ins, ins->out, note);
	    res += ((ins_node_ptr) ins->out->data)->output * note->volume;
	    i++;
	}
    }
    return res;
}

node_ptr ins_add_gen(ins_t ins, gen_info_t gi, char *id)
{
    node_ptr node;
    gen_ptr g;
    ins_node_ptr p;

    node = node_new();
    g = gen_new(gi);
    node->data = p = malloc(sizeof(ins_node_t));
    if (!strncmp("funk", gi->id, 4)) {
	p->type = ins_type_funk;
    } else {
	p->type = ins_type_normal;
    }
    p->id = strclone(id);
    p->gen = g;
    p->gen_index = ins->node_list->count;
    p->output = 0.0;

    darray_append(ins->node_list, node);

    if (gi->port_count > ins->maxport) {
	ins->maxport = g->info->port_count;
    }

    return node;
}

extern struct gen_info_s out_info;
extern struct gen_info_s dummy_info;

void ins_remove_all_notes(ins_t ins)
{
    int i;
    for (i=0; i<ins->note_list->count; i++) {
	ins_note_free(ins, (note_ptr) ins->note_list->item[i]);
    }
    darray_remove_all(ins->note_list);
}

void ins_init(ins_t ins, char *s)
{
    darray_init(ins->node_list);
    darray_init(ins->note_list);
    ins->id = s;

    ins->out = ins_add_gen(ins, &out_info, "out");
    ins->freq = ins_add_gen(ins, &dummy_info, "freq");
    ins->ref_count = 1;
    ins->maxport = 0;
}

ins_ptr ins_new(char *s)
{
    ins_ptr res;
    res = malloc(sizeof(ins_t));
    ins_init(res, s);
    return res;
}

void ins_clear(ins_ptr ins)
{
    int i, j;

    ins_remove_all_notes(ins);
    darray_clear(ins->note_list);

    //delete edges then nodes
    for (i=0; i<ins->node_list->count; i++) {
	node_ptr node = (node_ptr) ins->node_list->item[i];
	for (j=0; j<node->in->count; j++) {
	    edge_ptr e = (edge_ptr) node->in->item[j];
	    free(e->data);
	    free(e);
	}
    }
    for (i=0; i<ins->node_list->count; i++) {
	node_ptr node = (node_ptr) ins->node_list->item[i];
	ins_node_ptr p = (ins_node_ptr) node->data;
	free(p->id);
	gen_clear(p->gen);
	free(p->gen);
	node_clear(node);
	free(node->data);
	free(node);
    }
    darray_clear(ins->node_list);
}

static void update_ref_count(ins_t ins)
//count the number of nodes that are connected
//and have the ability to keep the note alive
{
    int j;

    void recurse_count(node_ptr node) {
	int i;
	ins_node_ptr p = (ins_node_ptr) node->data;
	if (p->gen->note_off_tick) ins->ref_count++;
	p->visited = 1;

	for (i=0; i<node->in->count; i++) {
	    edge_ptr e = node->in->item[i];
	    node_ptr n1 = e->src;
	    ins_node_ptr p1 = (ins_node_ptr) n1->data;

	    if (!p1->visited) {
		recurse_count(n1);
	    }
	}
    }

    ins->ref_count = 1;
    for (j=0; j<ins->node_list->count; j++) {
	node_ptr node = ins->node_list->item[j];
	((ins_node_ptr) node->data)->visited = 0;
    }
    recurse_count(ins->out);
}

edge_ptr ins_connect(ins_t ins, node_ptr src, node_ptr dst, int dstport)
{
    edge_ptr e;

    e = edge_new(src, dst);
    e->data = malloc(sizeof(int));
    *((int *) e->data) = dstport;
    update_ref_count(ins);
    return e;
}

void ins_disconnect(ins_t ins, edge_ptr e)
{
    free(e->data);
    edge_delete(e);
    update_ref_count(ins);
}

void set_param(node_ptr node, int n, double val)
{
    gen_ptr g = ((ins_node_ptr) node->data)->gen;

    g->param[n] = val;
    g->info->param[n]->callback(g, val);
}

int no_of_param(node_ptr node, char *id)
{
    int i;
    gen_ptr g = ((ins_node_ptr) node->data)->gen;
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
    ins_node_ptr inp;

    inp = (ins_node_ptr) node->data;
    return inp->type;
}

char *get_funk_program(node_ptr node)
{
    gen_ptr g;
    ins_node_ptr inp;

    inp = (ins_node_ptr) node->data;
    g = inp->gen;
    return funk_get_program(g);
}

void set_funk_program(node_ptr node, char *s)
{
    gen_ptr g;
    ins_node_ptr inp;

    inp = (ins_node_ptr) node->data;
    g = inp->gen;
    funk_clear_program(g);
    funk_init_program(g, s);
}
