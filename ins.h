#ifndef INS_H
#define INS_H

#include "darray.h"
#include "graph.h"
#include "note.h"
#include "gen.h"

struct ins_s {
    char *id;
    darray_t node_list;
    darray_t note_list;
    node_ptr out;
    node_ptr freq;
    int maxport;
    int ref_count;
};

typedef struct ins_s ins_t[1];
typedef struct ins_s *ins_ptr;

enum {
    ins_type_normal = 0,
    ins_type_funk,
};

struct ins_node_s {
    char *id;
    int gen_index;
    gen_ptr gen;
    int type;
    int visited;
    double output;
};

typedef struct ins_node_s ins_node_t[1];
typedef struct ins_node_s *ins_node_ptr;

edge_ptr ins_connect(ins_t ins, node_ptr src, node_ptr dst, int dstport);
void ins_init(ins_t ins, char *id);
ins_ptr ins_new(char *s);
void ins_clear(ins_ptr ins);
node_ptr ins_add_gen(ins_t ins, gen_info_t gi, char *id);
double ins_tick(ins_t ins);
note_ptr ins_note_on(ins_t ins, int noteno, double volume);
void ins_note_off(note_ptr n);
void ins_disconnect(ins_t ins, edge_ptr e);

void set_param(node_ptr node, int n, double val);
int no_of_param(node_ptr node, char *id);
void set_param_by_id(node_ptr node, char *id, double val);
void set_funk_program(node_ptr node, char *s);
int node_type(node_ptr node);
char *get_funk_program(node_ptr node);

#endif //INS_H
