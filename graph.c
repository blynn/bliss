#include "graph.h"

void graph_init(graph_ptr g)
{
    darray_init(g->vertex);
    darray_init(g->edge);
    darray_init(g->zorder);
}

void graph_clear(graph_ptr g)
{
    darray_clear(g->vertex);
    darray_clear(g->edge);
    darray_clear(g->zorder);
}
