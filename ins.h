#ifndef INS_H
#define INS_H

#include "voice.h"
#include "track.h"

struct ins_s {
    char *id;
    darray_t voicenode;
    graph_t graph;
    node_ptr out;
    darray_t gdlist; //gendata
    track_t track;
    darray_t gen_index_table;
};
typedef struct ins_s ins_t[1];
typedef struct ins_s *ins_ptr;

static inline ins_ptr node_get_ins(node_ptr node)
{
    node_data_ptr p = node->data;
    assert(p->type == node_type_ins);
    return p->ins;
}

void ins_init(ins_ptr ins, char *id);
ins_ptr ins_new();
void ins_clear(ins_ptr ins);
double ins_tick(ins_ptr ins);
void ins_note_on(ins_ptr ins, int noteno, double volume);
void ins_note_off(ins_ptr ins, int noteno);
node_ptr add_voice(char *id, ins_ptr ins, int x, int y);
node_ptr add_ins_unit(char *id, uentry_ptr u, ins_ptr ins, int x, int y);
#endif //INS_H
