#ifndef INS_H
#define INS_H

#include "voice.h"

struct ins_s {
    char *id;
    darray_t voicenode;
    graph_t graph;
    node_ptr out;
    darray_t gdlist;
};
typedef struct ins_s ins_t[1];
typedef struct ins_s *ins_ptr;

void ins_init(ins_ptr ins, char *id);
ins_ptr ins_new();
void ins_clear(ins_ptr ins);
node_ptr ins_add_voice(ins_ptr ins, char *id);
double ins_tick(ins_ptr ins);
void ins_note_on(ins_ptr ins, int noteno, double volume);
void ins_note_off(ins_ptr ins, int noteno);
#endif //INS_H
