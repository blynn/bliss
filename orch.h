#ifndef ORCH_H
#define ORCH_H

#include "ins.h"

struct orch_s {
    darray_t insnode;
    graph_t graph;
};
typedef struct orch_s orch_t[1];
typedef struct orch_s *orch_ptr;

void orch_init(orch_ptr orch);
void orch_clear(orch_ptr orch);
node_ptr orch_add_ins(orch_ptr orch, char *id, int x, int y);
double orch_tick(orch_ptr orch);
#endif //ORCH_H
