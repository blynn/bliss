#ifndef CANVAS_H
#define CANVAS_H

#include "layout.h"

enum {
    //vd = "vertex distance" (in pixels)
    //  +------+
    //  |      |     +-+
    //  | unit +-----| | <-- box representing output port
    //  |      |     +-+ (input ports are similar but on the left side)
    //  +------+
    vd_h = 13,
    vd_w = 80,
    vd_textpad = 2,
    vd_edgew = 4,
    vd_edgey = 6, //distance from top to out edge
    vd_porty = 3, //distance from top to box representing port
    vd_portw = 7, //width of box representing port
    vd_porth = 7, //height
    vd_fudge = 2, //tolerance for error when user goes for a port with the mouse
};

void canvas_select_nothing(widget_ptr canvas);
void canvas_select_connection(widget_ptr canvas, connection_ptr c);
void canvas_select_vertex(widget_ptr canvas, vertex_ptr v);
void canvas_put_layout(widget_ptr canvas, layout_ptr lp);
void canvas_init(widget_ptr canvas, widget_ptr parent,
	void (*select_nothing_func)(),
	void (*select_connection_func)(connection_ptr),
	void (*select_vertex_func)(vertex_ptr),
	void (*connection_func)(layout_ptr, vertex_ptr, vertex_ptr, int));

void canvas_placement_start(widget_ptr canvas,
	void (*callback)(void *, layout_ptr, int, int), void *data,
	int w, int h, char *s);
void canvas_placement_finish(widget_ptr canvas);
vertex_ptr canvas_current_vertex(widget_ptr canvas);
connection_ptr canvas_current_connection(widget_ptr canvas);
void canvas_remove_current_connection(widget_ptr canvas);
void canvas_remove_current_vertex(widget_ptr canvas);

#endif //CANVAS_H
