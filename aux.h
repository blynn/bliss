#ifndef AUX_H
#define AUX_H

#include "widget.h"

#include "graph.h"

void aux_put_geometry(int x, int y, int w, int h);
void aux_show_nothing();
void aux_show_node(node_ptr v);
void aux_show_edge(edge_ptr c);
void aux_init(widget_ptr parent);

#endif //AUX_H
