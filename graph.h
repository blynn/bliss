//generic graph data structure
#ifndef NODE_H
#define NODE_H

#include "darray.h"

struct graph_s;
typedef struct graph_s graph_t[1];
typedef struct graph_s *graph_ptr;

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

void node_init(node_t n, graph_ptr g);
node_ptr graph_add_node(graph_ptr g, void *data);
void graph_remove_node(graph_ptr g, node_ptr node);
edge_ptr edge_new(node_ptr src, node_ptr dst);
edge_ptr graph_add_edge(graph_ptr g, node_ptr src, node_ptr dst, void *data);
void edge_delete(edge_ptr e);

struct graph_s {
    darray_t node_list;
    darray_t edge_list;
    void (*new_node_cb)(node_ptr node, void *data);
    void *new_node_cb_data;
    void (*delete_node_cb)(node_ptr node, void *data);
    void *delete_node_cb_data;
};

static inline int graph_node_count(graph_ptr g)
{
    return g->node_list->count;
}

void graph_init(graph_ptr g);
void graph_clear(graph_ptr g);
void graph_forall_node(graph_ptr g, void (*func)(node_ptr));
void graph_put_new_node_cb(graph_ptr g, void (*func)(node_ptr, void *), void *data);
void graph_put_delete_node_cb(graph_ptr g, void (*func)(node_ptr, void *), void *data);
void graph_remove_edge(graph_ptr g, edge_ptr e);

#endif //NODE_H
