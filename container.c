#include <stdio.h>
#include <stdlib.h>
#include "container.h"

void container_moved(widget_ptr w, void *data)
{
    container_ptr c = (container_ptr) w;
    int i, n;

    n = c->child->count;
    for(i=0; i<n; i++) {
	widget_ptr wi = (widget_ptr) c->child->item[i];
	widget_notify_move(wi);
    }
}

int container_handle_event(widget_ptr w, event_ptr e)
{
    int i, n;
    container_ptr c = (container_ptr) w;

    n = c->child->count;
    for(i=0; i<n; i++) {
	widget_ptr wi = (widget_ptr) c->child->item[i];
	if (widget_handle_event(wi, e)) {
	    return 1;
	}
    }
    return 0;
}

void container_update(widget_ptr w)
{
    int i, n;
    container_ptr c = (container_ptr) w;
    n = c->child->count;

    for (i=0; i<n; i++) {
	widget_ptr wi = (widget_ptr) c->child->item[i];
	widget_update(wi);
    }
}

void container_put_widget(container_ptr con, widget_ptr w, int x, int y)
{
    widget_put_local(w, x, y);
    darray_append(con->child, w);
    w->parent = (widget_ptr) con;
}

void container_init(container_ptr c)
{
    widget_ptr w = c->widget;
    widget_init(w);
    darray_init(c->child);
    w->handle_event = container_handle_event;
    w->update = container_update;
    widget_connect(w, signal_move, container_moved, NULL);
}

void container_clear(container_ptr w)
{
    darray_clear(w->child);
    widget_clear(w->widget);
}

container_ptr container_new()
{
    container_ptr c;
    c = (container_ptr) malloc(sizeof(struct container_s));
    container_init(c);
    return c;
}
