//graph editor
#ifndef CANVAS_H
#define CANVAS_H

#include "utable.h"
#include "widget.h"
#include "graph.h"

void canvas_select_nothing(widget_ptr canvas);
void canvas_select_edge(widget_ptr canvas, edge_ptr e);
void canvas_select_node(widget_ptr canvas, node_ptr v);
void canvas_put_graph(widget_ptr canvas, graph_ptr g);
void canvas_init(widget_ptr canvas, widget_ptr parent);

void canvas_place_unit_start(widget_ptr canvas, uentry_ptr u);
void canvas_place_voice_start(widget_ptr canvas);
void canvas_place_ins_start(widget_ptr canvas);
void canvas_placement_finish(widget_ptr canvas);
node_ptr canvas_current_node(widget_ptr canvas);
edge_ptr canvas_current_edge(widget_ptr canvas);
void canvas_remove_current_edge(widget_ptr canvas);
void canvas_remove_current_node(widget_ptr canvas);

#endif //CANVAS_H
