//generic graph data structure
#ifndef NODE_H
#define NODE_H

#include "darray.h"

struct node_s {
    darray_t in;
    darray_t out;
    void *data;
};

typedef struct node_s node_t[1];
typedef struct node_s *node_ptr;

struct edge_s {
    node_ptr src, dst;
    void *data;
};

typedef struct edge_s edge_t[1];
typedef struct edge_s *edge_ptr;

void node_init(node_t n);
node_ptr node_new();
void node_clear(node_ptr n);
void node_delete(node_ptr n);
edge_ptr edge_new(node_ptr src, node_ptr dst);
void edge_delete(edge_ptr e);

#endif //NODE_H
