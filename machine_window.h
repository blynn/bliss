#ifndef MACHINE_WINDOW_H
#define MACHINE_WINDOW_H

#include "machine.h"
#include "widget.h"
#include "container.h"
#include "machine_area.h"
#include "sidebar.h"

struct machine_window_s {
    widget_t widget;
    container_t con;
    machine_area_t ma;
    sidebar_t sb;
};

typedef struct machine_window_s *machine_window_ptr;
typedef struct machine_window_s machine_window_t[1];

void machine_window_init(machine_window_ptr);
void machine_window_clear(machine_window_ptr ma);
machine_window_ptr machine_window_new();
void machine_window_edit(machine_window_ptr ma, song_ptr song);
void machine_window_center(machine_window_ptr ma, machine_ptr m);

#endif //MACHINE_WINDOW_H
