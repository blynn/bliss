#include <stdlib.h>
#include "unit_window.h"

enum {
    header_h = 20,
    sb_h = 80,
    padding = 5,
};

static int unit_window_handle_event(widget_ptr w, event_ptr e)
{
    unit_window_ptr uw = (unit_window_ptr) w;
    return widget_handle_event((widget_ptr) uw->con, e);
}

static void unit_window_update(widget_ptr w)
{
    unit_window_ptr uw = (unit_window_ptr) w;
    widget_update((widget_ptr) uw->con);
}

static void unit_window_moved(widget_ptr w, void *data)
{
    unit_window_ptr uw = (unit_window_ptr) w;
    widget_put_local((widget_ptr) uw->con, 0, 0);
}

static void unit_window_resize(widget_ptr w, void *data)
{
    unit_window_ptr uw = (unit_window_ptr) w;
    widget_put_size((widget_ptr) uw->con, w->w, w->h);
    widget_put_size((widget_ptr) uw->ua, w->w, w->h - sb_h - header_h - padding);
    widget_put_size((widget_ptr) uw->sb, w->w, sb_h);
    widget_put_local((widget_ptr) uw->sb, 0, w->h - sb_h);
}

void unit_window_init(unit_window_ptr uw)
{
    widget_ptr w = (widget_ptr) uw;

    widget_init(w);
    container_init(uw->con);
    unit_area_init(uw->ua);
    combobox_init(uw->cbmachine);
    unit_sidebar_init(uw->sb, uw->ua);
    container_put_widget(uw->con, (widget_ptr) uw->sb, 0, 0);
    container_put_widget(uw->con, (widget_ptr) uw->cbmachine, 16, 0);
    widget_put_size((widget_ptr) uw->cbmachine, 100, 16);
    container_put_widget(uw->con, (widget_ptr) uw->ua, 0, header_h);
    ((widget_ptr) uw->con)->parent = w;
    w->handle_event = unit_window_handle_event;
    w->update = unit_window_update;
    widget_connect(w, signal_resize, unit_window_resize, NULL);
    widget_connect(w, signal_move, unit_window_moved, NULL);
}

void unit_window_clear(unit_window_ptr uw)
{
    //TODO
}

unit_window_ptr unit_window_new()
{
    unit_window_ptr r = (unit_window_ptr) malloc(sizeof(struct unit_window_s));
    unit_window_init(r);
    return r;
}

void unit_window_edit(unit_window_ptr uw, machine_info_ptr mi)
{
    unit_area_edit(uw->ua, mi);
}

void unit_window_center(unit_window_ptr uw, unit_ptr m)
{
    unit_area_center(uw->ua, m);
}
