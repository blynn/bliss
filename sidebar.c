#include <stdlib.h>
#include "sidebar.h"

static int sidebar_handle_event(widget_ptr w, event_ptr e)
{
    return 0;
}

static void sidebar_update(widget_ptr w)
{
    char buf[128];
    sidebar_ptr sb = (sidebar_ptr) w;
    machine_area_ptr ma = sb->ma;
    machine_ptr m = ma->sel_machine;
    edge_ptr e = ma->sel_edge;

    widget_draw_inverse_border(w);
    if (e) {
	widget_write(w, 3, 3, "Connection");

	sprintf(buf, "From: %s", e->src->id);
	widget_write(w, 13, 23, buf);
	sprintf(buf, "To: %s", e->dst->id);
	widget_write(w, 13, 43, buf);

    } else if (m) {
	/*
	char *s;
	switch(m->mi->type) {
	    case machine_generator:
		s = "Generator";
		break;
	    case machine_effect:
		s = "Effect";
		break;
	    case machine_master:
		s = "Master";
		break;
	}
	widget_write(w, 3, 3, s);
	*/
	sprintf(buf, "%s", m->id);
	widget_write(w, 3, 3, buf);
	sprintf(buf, "%s", m->mi->id);
	widget_write(w, 3, 23, buf);
    } else {
    }
}

void sidebar_init(sidebar_ptr sb, machine_area_ptr ma)
{
    widget_ptr w = (widget_ptr) sb;

    widget_init(w);
    sb->ma = ma;

    w->handle_event = sidebar_handle_event;
    w->update = sidebar_update;
}

void sidebar_clear(sidebar_ptr sb)
{
    //TODO
}

sidebar_ptr sidebar_new(machine_area_ptr ma)
{
    sidebar_ptr r = (sidebar_ptr) malloc(sizeof(struct sidebar_s));
    sidebar_init(r, ma);
    return r;
}
