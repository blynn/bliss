#include <stdlib.h>

#include "graph.h"

void node_init(node_t n)
{
    darray_init(n->in);
    darray_init(n->out);
}

node_ptr node_new()
{
    node_ptr res = malloc(sizeof(node_t));
    node_init(res);
    return res;
}

void node_clear(node_ptr node)
{
    darray_clear(node->in);
    darray_clear(node->out);
}

void node_delete(node_ptr node)
{
    node_clear(node);
    free(node);
}

edge_ptr edge_new(node_ptr src, node_ptr dst)
{
    edge_ptr res;

    res = malloc(sizeof(edge_t));
    res->src = src;
    res->dst = dst;

    darray_append(src->out, res);
    darray_append(dst->in, res);

    return res;
}

void edge_delete(edge_ptr e)
{
    darray_remove(e->src->out, e);
    darray_remove(e->dst->in, e);
    free(e);
}
