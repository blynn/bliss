#ifndef UNIT_WINDOW_H
#define UNIT_WINDOW_H

#include "machine.h"
#include "widget.h"
#include "container.h"
#include "unit_area.h"
#include "unit_sidebar.h"

struct unit_window_s {
    widget_t widget;
    container_t con;
    unit_area_t ua;
    unit_sidebar_t sb;
    combobox_t cbmachine;
};

typedef struct unit_window_s *unit_window_ptr;
typedef struct unit_window_s unit_window_t[1];

void unit_window_init(unit_window_ptr);
void unit_window_clear(unit_window_ptr ua);
unit_window_ptr unit_window_new();
void unit_window_edit(unit_window_ptr ua, machine_info_ptr mi);
void unit_window_center(unit_window_ptr ua, unit_ptr u);

#endif //UNIT_WINDOW_H
