#include <stdlib.h>
#include <string.h>
#include "ins.h"
#include "util.h"

#include <assert.h>

extern gen_info_t out_info;

static void ins_del_node(node_ptr node, void *data)
{
    node_data_ptr p = node->data;
    ins_ptr ins = data;
    int i = p->gen_index;
    node_ptr last_node;

    gen_data_ptr gdp = ins->gdlist->item[i];
    gen_note_free(p->gen, gdp->data);
    free(gdp);
    free(p);

    last_node = darray_last(ins->gen_index_table);
    if (node != last_node) {
	node_data_ptr lndp = last_node->data;
	darray_put(ins->gen_index_table, last_node, i);
	lndp->gen_index = i;
	darray_put(ins->gdlist, darray_last(ins->gdlist), i);
    }
    darray_remove_last(ins->gen_index_table);
    darray_remove_last(ins->gdlist);
}

static void ins_del_voice(node_ptr node, void *data)
{
    node_data_ptr p = node->data;
    ins_ptr ins = data;

    darray_remove(ins->voicenode, node);
    voice_free(p->voice);
    free(p);
}

void ins_init(ins_ptr ins, char *id)
{
    darray_init(ins->voicenode);
    darray_init(ins->gdlist);
    darray_init(ins->gen_index_table);
    graph_init(ins->graph);
    ins->id = strclone(id);
    ins->out = NULL;
    track_init(ins->track, "track0");
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
    graph_forall_node(ins->graph, node_self_clear);
    graph_clear(ins->graph);

    assert(darray_is_empty(ins->voicenode));
    assert(darray_is_empty(ins->gdlist));
    darray_clear(ins->voicenode);
    darray_clear(ins->gdlist);
    track_clear(ins->track);
}

node_ptr add_ins_unit(char *id, uentry_ptr u, ins_ptr ins, int x, int y)
{
    node_ptr node = node_from_gen_info(ins->graph, u->info, id);
    node_data_ptr p = node->data;
    node->x = x;
    node->y = y;

    p->gen_index = ins->gen_index_table->count;
    gen_data_ptr gdp = malloc(sizeof(gen_data_t));
    darray_append(ins->gdlist, gdp);
    darray_append(ins->gen_index_table, node);
    gdp->data = gen_note_on(p->gen);

    p->clear = ins_del_node;
    p->clear_data = ins;
    return node;
}

node_ptr add_voice(char *id, ins_ptr ins, int x, int y)
{
    node_data_ptr p;
    voice_ptr voice;
    node_ptr node;

    voice = voice_new(id);

    p = malloc(sizeof(node_data_t));
    p->type = node_type_voice;
    p->voice = voice;
    p->id = voice->id;

    p->clear = ins_del_voice;
    p->clear_data = ins;
    node = graph_add_node(ins->graph, p);
    node->x = x;
    node->y = y;

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
