//2D graph layout

#ifndef GRAPH_H
#define GRAPH_H

#include "darray.h"

struct graph_s {
    darray_t vertex;
    darray_t edge;
    darray_t zorder;
};
typedef struct graph_s *graph_ptr;
typedef struct graph_s graph_t[1];

struct vertex_s {
    darray_t in;
    darray_t out;
    int x, y;
    void *data;
};
typedef struct vertex_s *vertex_ptr;
typedef struct vertex_s vertex_t[1];

struct edge_s {
    vertex_ptr src, dst;
    int x, y;
    void *data;
};
typedef struct edge_s *edge_ptr;
typedef struct edge_s edge_t[1];

void graph_init(graph_ptr g);
void graph_clear(graph_ptr g);

#endif //GRAPH_H
