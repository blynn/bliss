#include <stdlib.h>
#include "layout.h"

void layout_init(layout_ptr p, widget_ptr parent, void *data)
{
    p->parent = parent;
    darray_init(p->vertex_list);
    darray_init(p->connection_list);
    p->data = data;
}

void layout_clear(layout_ptr p)
{
    int i;

    for (i=0; i<p->vertex_list->count; i++) {
	vertex_ptr v = (vertex_ptr) p->vertex_list->item[i];
	vertex_free(v);
    }
    for (i=0; i<p->connection_list->count; i++) {
	connection_ptr c = (connection_ptr) p->connection_list->item[i];
	connection_free(c);
    }
    darray_clear(p->vertex_list);
    darray_clear(p->connection_list);
}

layout_ptr layout_new(widget_ptr parent, void *data)
{
    layout_ptr res = (void *) malloc(sizeof(layout_t));
    layout_init(res, parent, data);
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

void vertex_free(vertex_ptr v)
{
    widget_clear(v->w);
    free(v);
}

void layout_remove_vertex(layout_ptr lp, vertex_ptr v)
{
    //TODO: remove connections
    darray_remove(lp->vertex_list, v);
    vertex_free(v);
}

void connection_free(connection_ptr c)
{
    widget_clear(c->w);
    free(c);
}
