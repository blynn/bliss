#include <stdlib.h>
#include "layout.h"

void layout_init(layout_ptr lp, widget_ptr parent, graph_ptr graph)
{
    lp->parent = parent;
    darray_init(lp->vertex_list);
    darray_init(lp->connection_list);
    lp->graph = graph;
    htable_init(lp->etab);
}

void layout_clear(layout_ptr lp)
{
    darray_ptr a = lp->vertex_list;
    while (a->count) {
	vertex_ptr v = darray_last(a);
	layout_remove_vertex(lp, v);
    }

    darray_clear(lp->vertex_list);
    darray_clear(lp->connection_list);
    htable_clear(lp->etab);
}

layout_ptr layout_new(widget_ptr parent, graph_ptr graph)
{
    layout_ptr res = (void *) malloc(sizeof(layout_t));
    layout_init(res, parent, graph);
    return res;
}

void layout_free(layout_ptr lp)
{
    layout_clear(lp);
    free(lp);
}

vertex_ptr vertex_new(layout_ptr lp)
{
    vertex_ptr res;
    res = malloc(sizeof(vertex_t));
    widget_init(res->w, lp->parent);
    return res;
}

void layout_remove_vertex(layout_ptr lp, vertex_ptr v)
{
    //remove in/out connections
    darray_ptr a = v->node->in;
    while (a->count) {
	edge_ptr e = darray_last(a);
	connection_ptr c = htable_at(lp->etab, e);
	layout_remove_connection(lp, c);
    }

    a = v->node->out;
    while (a->count) {
	edge_ptr e = darray_last(a);
	connection_ptr c = htable_at(lp->etab, e);
	layout_remove_connection(lp, c);
    }

    //remove vertex
    darray_remove(lp->vertex_list, v);
    graph_remove_node(lp->graph, v->node);
    widget_clear(v->w);
    free(v);
}

connection_ptr layout_add_connection(layout_ptr lp,
	vertex_ptr src, vertex_ptr dst, void *data)
{
    connection_ptr c = (connection_ptr) malloc(sizeof(connection_t));

    widget_init(c->w, lp->parent);
    c->src = src;
    c->dst = dst;
    c->edge = graph_add_edge(lp->graph, src->node, dst->node, data);
    darray_append(lp->connection_list, c);
    htable_put(lp->etab, c, c->edge);
    return c;
}

void layout_remove_connection(layout_ptr lp, connection_ptr c)
{
    htable_remove(lp->etab, c->edge);
    darray_remove(lp->connection_list, c);
    widget_clear(c->w);
    graph_remove_edge(lp->graph, c->edge);
    free(c);
}
