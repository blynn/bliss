#ifndef LAYOUT_H
#define LAYOUT_H

#include "widget.h"
#include "graph.h"
#include "htable.h"

//vertex = widgetized node
//also keeps count of ports
struct vertex_s {
    widget_t w;
    node_ptr node;
    int inportcount;
};

typedef struct vertex_s vertex_t[1];
typedef struct vertex_s* vertex_ptr;

//connection keeps tracks of edges
//holds vertices as well for convenience
struct connection_s {
    widget_t w;
    vertex_ptr src, dst;
    edge_ptr edge;
};

typedef struct connection_s connection_t[1];
typedef struct connection_s *connection_ptr;

struct layout_s {
    widget_ptr parent;
    darray_t vertex_list; //list of vertices sorted by z-order
    darray_t connection_list;
    graph_ptr graph; //graph that this is a layout of
    htable_t etab; //connections indexed by edges
};
typedef struct layout_s layout_t[1];
typedef struct layout_s *layout_ptr;

void layout_init(layout_ptr p, widget_ptr parent, graph_ptr graph);
void layout_clear(layout_ptr p);
void layout_free(layout_ptr p);
layout_ptr layout_new(widget_ptr parent, graph_ptr graph);
void layout_remove_connection(layout_ptr lp, connection_ptr c);
connection_ptr layout_add_connection(layout_ptr lp,
	vertex_ptr src, vertex_ptr dst, void *data);

vertex_ptr vertex_new(layout_ptr lp);
void layout_remove_vertex(layout_ptr lp, vertex_ptr v);

#endif //LAYOUT_H
