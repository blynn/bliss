#include <stdlib.h>
#include <string.h>
#include "ins.h"
#include "util.h"

#include <assert.h>

extern gen_info_t out_info;

static void ins_new_node(node_ptr node, void *data)
{
    node_data_ptr p = node->data;
    ins_ptr ins = data;

    if (p->type == node_type_funk || p->type == node_type_normal) {
	gen_data_ptr gdp = malloc(sizeof(gen_data_t));
	darray_append(ins->gdlist, gdp);
	gdp->data = gen_note_on(p->gen);
    } else {
	//TODO: very messy: can't I avoid?
	darray_append(ins->gdlist, NULL);
    }
}

static void ins_delete_node(node_ptr node, void *data)
{
    node_data_ptr p = node->data;
    ins_ptr ins = data;
    int i = p->gen_index;

    if (p->type == node_type_funk || p->type == node_type_normal) {
	gen_data_ptr gdp = ins->gdlist->item[i];
	gen_note_free(p->gen, gdp->data);
	free(gdp);
    } else if (p->type == node_type_voice) {
	darray_remove(ins->voicenode, node);
	voice_free(p->voice);
    }
    free(p);

    {
	//TODO: move last to i instead?
	void adjust_index(node_ptr node) {
	    node_data_ptr ndp = node->data;
	    if (ndp->gen_index > i) ndp->gen_index--;
	}

	graph_forall_node(ins->graph, adjust_index);
	darray_remove_index(ins->gdlist, i);
    }
}

void ins_init(ins_ptr ins, char *id)
{
    darray_init(ins->voicenode);
    darray_init(ins->gdlist);
    graph_init(ins->graph);
    graph_put_new_node_cb(ins->graph, ins_new_node, ins);
    graph_put_delete_node_cb(ins->graph, ins_delete_node, ins);
    ins->out = node_from_gen_info(ins->graph, out_info, "out");
    ins->id = strclone(id);
}

ins_ptr ins_new(char *id)
{
    ins_ptr res = (ins_ptr) malloc(sizeof(ins_t));
    ins_init(res, id);
    return(res);
}

void ins_clear(ins_ptr ins)
{
    free(ins->id);
    graph_clear(ins->graph);

    //voicenode and gdlist are freed during ins_delete_node() callbacks
    assert(darray_is_empty(ins->voicenode));
    assert(darray_is_empty(ins->gdlist));
    darray_clear(ins->voicenode);
    darray_clear(ins->gdlist);

}

static node_ptr node_from_voice(ins_ptr ins, voice_ptr voice)
{
    node_data_ptr p;

    p = malloc(sizeof(node_data_t));
    p->type = node_type_voice;
    p->voice = voice;
    p->id = voice->id; //TODO: get rid of node->id? voice has it anyway
    //TODO: hackish:
    p->gen_index = ins->graph->node_list->count;
    return graph_add_node(ins->graph, p);
}

node_ptr ins_add_voice(ins_ptr ins, char *id)
{
    node_ptr node = node_from_voice(ins, voice_new(id));
    darray_append(ins->voicenode, node);
    return node;
}

void ins_note_on(ins_ptr ins, int noteno, double volume)
{
    int i;
    for (i=0; i<ins->voicenode->count; i++) {
	node_ptr node = ins->voicenode->item[i];
	voice_ptr v = ((node_data_ptr) node->data)->voice;
	if (v->notemin <= noteno && v->notemax >= noteno) {
	    voice_note_on(v, noteno, volume);
	}
    }
}

void ins_note_off(ins_ptr ins, int noteno)
{
    int i;
    for (i=0; i<ins->voicenode->count; i++) {
	node_ptr node = ins->voicenode->item[i];
	voice_ptr v = ((node_data_ptr) node->data)->voice;
	if (v->notemin <= noteno && v->notemax >= noteno) {
	    voice_note_off(v, noteno);
	}
    }
}

static void recurse_tick(ins_ptr ins, node_ptr node)
{
    int i;
    node_data_ptr p = node->data;
    gen_data_ptr gdp = ins->gdlist->item[p->gen_index];
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

	if (!p1->visited) recurse_tick(ins, n1);
	invalue[portno] += p1->output;
    }

    //compute output value
    p->output = gen_tick(p->gen, gdp, invalue);
}

double ins_tick(ins_ptr ins)
{
    node_ptr outnode = ins->out;
    node_data_ptr outp = outnode->data;
    int i;
    void clear_visited(node_ptr node) {
	((node_data_ptr) node->data)->visited = 0;
    }

    graph_forall_node(ins->graph, clear_visited);

    //TODO: use a forall
    for (i=0; i<ins->voicenode->count; i++) {
	node_ptr node = ins->voicenode->item[i];
	node_data_ptr p = node->data;
	p->visited = -1;
	p->output = voice_tick(p->voice);
    }

    recurse_tick(ins, outnode);
    return outp->output;
}
