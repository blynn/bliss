#ifndef VOICE_H
#define VOICE_H

#include "darray.h"
#include "graph.h"
#include "note.h"
#include "gen.h"

struct voice_s {
    char *id;
    darray_t node_list;
    darray_t note_list;
    node_ptr out;
    node_ptr freq;
    int maxport;
    int ref_count;
    note_ptr keyboard[128];
    int notemin;
    int notemax;
};

typedef struct voice_s voice_t[1];
typedef struct voice_s *voice_ptr;

enum {
    node_type_normal = 0,
    node_type_funk,
    node_type_voice,
};

struct node_data_s {
    char *id;
    int type;
    int visited;
    double output;
    int gen_index;
    gen_ptr gen;
    voice_ptr voice;
};

typedef struct node_data_s node_data_t[1];
typedef struct node_data_s *node_data_ptr;

edge_ptr voice_connect(voice_t ins, node_ptr src, node_ptr dst, int dstport);
void voice_init(voice_t ins, char *id);
voice_ptr voice_new(char *s);
void voice_clear(voice_ptr ins);
void voice_free(voice_ptr voice);
node_ptr voice_add_gen(voice_t ins, gen_info_t gi, char *id);
double voice_tick(voice_t ins);
note_ptr voice_note_on(voice_t ins, int noteno, double volume);
void voice_note_off(voice_t voice, int noteno);
void voice_disconnect(voice_t ins, edge_ptr e);

void set_param(node_ptr node, int n, double val);
int no_of_param(node_ptr node, char *id);
void set_param_by_id(node_ptr node, char *id, double val);
void set_funk_program(node_ptr node, char *s);
int node_type(node_ptr node);
char *get_funk_program(node_ptr node);

#endif //VOICE_H
