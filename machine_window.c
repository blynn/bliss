#include <stdlib.h>
#include "machine_window.h"

static int machine_window_handle_event(widget_ptr w, event_ptr e)
{
    machine_window_ptr mw = (machine_window_ptr) w;
    return widget_handle_event((widget_ptr) mw->con, e);
}

static void machine_window_update(widget_ptr w)
{
    machine_window_ptr mw = (machine_window_ptr) w;
    widget_update((widget_ptr) mw->con);
}

static void machine_window_moved(widget_ptr w, void *data)
{
    machine_window_ptr mw = (machine_window_ptr) w;
    widget_put_local((widget_ptr) mw->con, 0, 0);
}

static void machine_window_resize(widget_ptr w, void *data)
{
    machine_window_ptr mw = (machine_window_ptr) w;
    widget_put_size((widget_ptr) mw->con, w->w, w->h);
    widget_put_size((widget_ptr) mw->sb, 128 - 2, w->h);
    widget_put_size((widget_ptr) mw->ma, w->w - 128, w->h);
}

void machine_window_init(machine_window_ptr mw)
{
    widget_ptr w = (widget_ptr) mw;

    widget_init(w);
    container_init(mw->con);
    machine_area_init(mw->ma);
    sidebar_init(mw->sb, mw->ma);
    container_put_widget(mw->con, (widget_ptr) mw->sb, 0, 0);
    container_put_widget(mw->con, (widget_ptr) mw->ma, 128, 0);
    ((widget_ptr) mw->con)->parent = w;
    w->handle_event = machine_window_handle_event;
    w->update = machine_window_update;
    widget_connect(w, signal_resize, machine_window_resize, NULL);
    widget_connect(w, signal_move, machine_window_moved, NULL);
}

void machine_window_clear(machine_window_ptr mw)
{
    //TODO
}

machine_window_ptr machine_window_new()
{
    machine_window_ptr r = (machine_window_ptr) malloc(sizeof(struct machine_window_s));
    machine_window_init(r);
    return r;
}

void machine_window_edit(machine_window_ptr mw, song_ptr song)
{
    machine_area_edit(mw->ma, song);
}

void machine_window_center(machine_window_ptr mw, machine_ptr m)
{
    machine_area_center(mw->ma, m);
}

void machine_window_put_buzz_coord(machine_window_ptr mw, machine_ptr m, double x, double y)
{
    machine_area_put_buzz_coord(mw->ma, m, x, y);
}
