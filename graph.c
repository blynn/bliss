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
    res->x = res->y = 0;
    return res;
}

void graph_remove_node(graph_ptr g, node_ptr node)
{
    void remove_edge(void *data) {
	graph_remove_edge(g, (edge_ptr) data);
    }
    //remove in/out connections
    darray_forall(node->in, remove_edge);
    darray_forall(node->out, remove_edge);

    darray_remove(g->node_list, node);

    free(node);
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

    darray_append(g->edge_list, res);
    return res;
}

void graph_remove_edge(graph_ptr g, edge_ptr e)
{
    darray_remove(e->src->out, e);
    darray_remove(e->dst->in, e);
    darray_remove(g->edge_list, e);
    free(e);
}

void graph_clear(graph_ptr g)
{
    void free_node(void *data) {
	node_ptr node = data;
	darray_clear(node->in);
	darray_clear(node->out);
    }
    void free_edge(void *data) {
    }
    darray_forall(g->node_list, free_node);
    darray_clear(g->node_list);
    darray_forall(g->node_list, free_edge);
    darray_clear(g->edge_list);
}

void graph_init(graph_ptr g)
{
    darray_init(g->node_list);
    darray_init(g->edge_list);
}

void graph_forall_node(graph_ptr g, void (*func)(node_ptr))
{
    void do_func(void *data) {
	func((node_ptr) data);
    }

    darray_forall(g->node_list, do_func);
}
