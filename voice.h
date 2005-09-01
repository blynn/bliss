#ifndef VOICE_H
#define VOICE_H

#include <assert.h>

#include "darray.h"
#include "graph.h"
#include "note.h"
#include "gen.h"
#include "utable.h"

struct voice_s {
    char *id;
    graph_t graph;
    darray_t note_list;
    node_ptr out;
    note_ptr keyboard[128];
    int notemin;
    int notemax;
    darray_t gen_index_table;
};

typedef struct voice_s voice_t[1];
typedef struct voice_s *voice_ptr;

enum {
    node_type_unit = 0,
    node_type_voice,
    node_type_ins,
};

struct ins_s;

struct node_data_s {
    char *id;
    int type;
    int visited;
    double output; //one output per node; polyphony + feedback doesn't work
    void (*clear)(node_ptr, void *);
    void *clear_data;
    union {
	voice_ptr voice;
	struct ins_s *ins;
	struct {
	    gen_ptr gen;
	    int gen_index;
	};
    };
};

typedef struct node_data_s node_data_t[1];
typedef struct node_data_s *node_data_ptr;

edge_ptr voice_connect(voice_ptr voice, node_ptr src, node_ptr dst, int dstport);
void voice_init(voice_t, char *id);
voice_ptr voice_new(char *s);
void voice_clear(voice_ptr);
void voice_free(voice_ptr voice);
node_ptr node_from_gen_info(graph_ptr graph, gen_info_t gi, char *id);
double voice_tick(voice_t);
note_ptr voice_note_on(voice_t, int noteno, double volume);
void voice_note_off(voice_t voice, int noteno);

void set_param(node_ptr node, int n, double val);
int no_of_param(node_ptr node, char *id);
void set_funk_program(node_ptr node, char *s);
int node_type(node_ptr node);
char *get_funk_program(node_ptr node);

static inline voice_ptr node_get_voice(node_ptr node)
{
    node_data_ptr p = node->data;
    assert(p->type == node_type_voice);
    return p->voice;
}

node_ptr voice_add_unit(voice_ptr voice, char *id, uentry_ptr u, int x, int y);
void node_self_clear(node_ptr node);

edge_ptr add_edge(graph_ptr g, node_ptr src, node_ptr dst, int port);

#endif //VOICE_H
