#include <string.h>
#include "unit_sidebar.h"
#include "unit_area.h"

static int unit_sidebar_handle_event(widget_ptr w, event_ptr e)
{
    unit_sidebar_ptr sb = (unit_sidebar_ptr) w;
    return widget_handle_event((widget_ptr) sb->con, e);
}

static void unit_sidebar_moved(widget_ptr w, void *data)
{
    unit_sidebar_ptr sb = (unit_sidebar_ptr) w;
    widget_put_local((widget_ptr) sb->con, 0, 0);
}

static void unit_sidebar_update(widget_ptr w)
{
    char *s;
    unit_sidebar_ptr sb = (unit_sidebar_ptr) w;
    unit_area_ptr ua = sb->ua;
    widget_draw_inverse_border(w);
    unit_ptr u;
    unit_edge_ptr e;
    u = ua->sel_unit;
    e = ua->sel_edge;
widget_hide((widget_ptr) sb->cbinport);
widget_hide((widget_ptr) sb->cboutport);
    if (e) {
	int i;

	widget_write(w, 3, 3, "From:");
	widget_write(w, 200, 3, "Port:");
	widget_write(w, 3, 23, "To:");
	widget_write(w, 200, 23, "Port:");
	widget_write(w, 40, 3, e->src->id);
	widget_write(w, 40, 23, e->dst->id);
	switch(e->src->type) {
	    case ut_plugin:
//TODO: move to separate function
darray_remove_all(sb->inlist);
for (i=0; i<e->src->ui->port_count; i++) {
    port_desc_ptr p = (port_desc_ptr) &e->src->ui->port_desc[i];
    if (p->type == up_output) {
	darray_append(sb->inlist, p->id);
    }
}
sb->cbinport->choice = sb->inlist;
widget_show((widget_ptr) sb->cbinport);
		s = e->src->ui->port_desc[e->srcport].id;
		combobox_put_text(sb->cbinport, s);
		break;
	    case ut_stream:
		s = "<input stream>";
		widget_write(w, 240, 3, s);
		break;
	    case ut_param:
		s = "<parameter>";
		widget_write(w, 240, 3, s);
		break;
	    default:
		s = "<unhandled!>";
		widget_write(w, 240, 3, s);
		break;
	}
	switch(e->dst->type) {
	    case ut_plugin:
//TODO: move to separate function
darray_remove_all(sb->outlist);
for (i=0; i<e->dst->ui->port_count; i++) {
    port_desc_ptr p = (port_desc_ptr) &e->dst->ui->port_desc[i];
    if (p->type == up_input) {
	darray_append(sb->outlist, p->id);
    }
}
sb->cboutport->choice = sb->outlist;
widget_show((widget_ptr) sb->cboutport);
		s = e->dst->ui->port_desc[e->dstport].id;
		combobox_put_text(sb->cboutport, s);
		break;
	    case ut_stream:
		s = "<output stream>";
		widget_write(w, 240, 23, s);
		break;
	    default:
		s = "<unhandled!>";
		widget_write(w, 240, 23, s);
		break;
	}
    }
    widget_update((widget_ptr) sb->con);
}

void unit_sidebar_clear(unit_sidebar_ptr sb)
{
    //TODO: finish this off
    widget_ptr w = sb->widget;
    combobox_clear(sb->cbinport);
    combobox_clear(sb->cboutport);
    container_clear(sb->con);
    widget_clear(w);
}

static void set_inport(widget_ptr caller, void *data)
{
    unit_area_ptr ua = (unit_area_ptr) data;
    combobox_ptr cb = (combobox_ptr) caller;
    unit_edge_ptr e = ua->sel_edge;
    int i;
    for (i=0; i<e->src->ui->port_count; i++) {
	port_desc_ptr p = (port_desc_ptr) &e->src->ui->port_desc[i];
	if (!strcmp(p->id, cb->text)) {
	    ua->sel_edge->srcport = i;
	    return;
	}
    }
}

static void set_outport(widget_ptr caller, void *data)
{
    unit_area_ptr ua = (unit_area_ptr) data;
    combobox_ptr cb = (combobox_ptr) caller;
    unit_edge_ptr e = ua->sel_edge;
    int i;
    for (i=0; i<e->dst->ui->port_count; i++) {
	port_desc_ptr p = (port_desc_ptr) &e->dst->ui->port_desc[i];
	if (!strcmp(p->id, cb->text)) {
	    ua->sel_edge->dstport = i;
	    return;
	}
    }
}

void unit_sidebar_init(unit_sidebar_ptr sb, unit_area_ptr ua)
{
    widget_ptr w = sb->widget;

    widget_init(w);
    sb->ua = ua;
    w->update = unit_sidebar_update;
    w->handle_event = unit_sidebar_handle_event;
    widget_connect(w, signal_move, unit_sidebar_moved, NULL);
    container_init(sb->con);
    combobox_init(sb->cbinport);
    combobox_init(sb->cboutport);
    container_put_widget(sb->con, (widget_ptr) sb->cbinport, 240, 3);
    container_put_widget(sb->con, (widget_ptr) sb->cboutport, 240, 23);
    widget_put_size((widget_ptr) sb->cbinport, 100, 16);
    widget_put_size((widget_ptr) sb->cboutport, 100, 16);
    ((widget_ptr) sb->con)->parent = w;
    darray_init(sb->inlist);
    darray_init(sb->outlist);
    widget_connect((widget_ptr) sb->cbinport, signal_activate, set_inport, ua);
    widget_connect((widget_ptr) sb->cboutport, signal_activate, set_outport, ua);
}
