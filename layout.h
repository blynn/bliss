#ifndef LAYOUT_H
#define LAYOUT_H

#include "widget.h"
#include "graph.h"

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
    void *data;
};
typedef struct layout_s layout_t[1];
typedef struct layout_s *layout_ptr;

void layout_init(layout_ptr p, widget_ptr parent, void *data);
void layout_clear(layout_ptr p);
void layout_free(layout_ptr p);
layout_ptr layout_new(widget_ptr parent, void *data);
void connection_free(connection_ptr c);

vertex_ptr vertex_new(layout_ptr lp);
void vertex_free(vertex_ptr v);
void layout_remove_vertex(layout_ptr lp, vertex_ptr v);

#endif //LAYOUT_H
