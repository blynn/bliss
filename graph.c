#include <stdlib.h>
#include <assert.h>

#include "graph.h"

node_ptr graph_add_node(graph_ptr g, void *data)
{
    node_ptr res = malloc(sizeof(node_t));

    darray_init(res->in);
    darray_init(res->out);
    darray_append(g->node_list, res);
    res->data = data;
    if (g->new_node_cb) {
	g->new_node_cb(res, g->new_node_cb_data);
    }
    return res;
}

void graph_remove_node(graph_ptr g, node_ptr node)
{
    //TODO: remove edges
    //(doesn't matter if remove_vertex called from layout.c
    //for completeness should do it here too)

    darray_remove(g->node_list, node);

    if (g->delete_node_cb) {
	g->delete_node_cb(node, g->delete_node_cb_data);
    }
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

edge_ptr graph_add_edge(graph_ptr g, node_ptr src, node_ptr dst, void *data)
{
    edge_ptr res;

    res = malloc(sizeof(edge_t));
    res->src = src;
    res->dst = dst;
    res->data = data;

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

void graph_remove_edge(graph_ptr g, edge_ptr e)
{
    free(e->data); //TODO: do this in callback
    edge_delete(e);
}

void graph_clear(graph_ptr g)
{
    //TODO: memory leaks galore in general
    if (!darray_is_empty(g->node_list)) {
	assert(0 && "graph not empty : memory leaks\n");
    }
    darray_clear(g->node_list);
    darray_clear(g->edge_list);
}

void graph_init(graph_ptr g)
{
    darray_init(g->node_list);
    darray_init(g->edge_list);
    g->new_node_cb = NULL;
    g->delete_node_cb = NULL;
}

void graph_forall_node(graph_ptr g, void (*func)(node_ptr))
{
    void do_func(void *data) {
	func((node_ptr) data);
    }

    darray_forall(g->node_list, do_func);
}

void graph_put_new_node_cb(graph_ptr g, void (*func)(node_ptr, void *), void *data)
{
    g->new_node_cb = func;
    g->new_node_cb_data = data;
}

void graph_put_delete_node_cb(graph_ptr g, void (*func)(node_ptr, void *), void *data)
{
    g->delete_node_cb = func;
    g->delete_node_cb_data = data;
}
