#ifndef GRAPH_H
#define GRAPH_H

#include "darray.h"

struct graph_s;
typedef struct graph_s *graph_ptr;

struct node_s {
    darray_t in;
    darray_t out;
    void *data;
    int x, y;
};

typedef struct node_s node_t[1];
typedef struct node_s *node_ptr;

struct edge_s {
    node_ptr src, dst;
    void *data;
};

typedef struct edge_s edge_t[1];
typedef struct edge_s *edge_ptr;

void node_init(node_t n, graph_ptr g);
node_ptr graph_add_node(graph_ptr g, void *data);
void graph_remove_node(graph_ptr g, node_ptr node);
edge_ptr graph_add_edge(graph_ptr g, node_ptr src, node_ptr dst, void *data);

struct graph_s {
    darray_t node_list;
    darray_t edge_list;
};
typedef struct graph_s graph_t[1];

static inline int graph_node_count(graph_ptr g)
{
    return g->node_list->count;
}

void graph_init(graph_ptr g);
void graph_clear(graph_ptr g);
void graph_forall_node(graph_ptr g, void (*func)(node_ptr));
void graph_remove_edge(graph_ptr g, edge_ptr e);

#endif //GRAPH_H
