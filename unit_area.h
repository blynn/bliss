#ifndef UNIT_AREA_H
#define UNIT_AREA_H

#include "machine.h"
#include "widget.h"
#include "menu.h"
#include "song.h"
#include "tbwin.h"

struct unit_area_s {
    widget_t widget;
    int drag_flag;
    int drag_startx;
    int drag_starty;
    machine_info_ptr mi;
    unit_ptr sel_unit;
    unit_edge_ptr sel_edge;
    menu_t machmenu;
    menu_t edgemenu;
    menu_t rootmenu;
    menu_t listmenu;
    darray_t zorder;
    tbwin_t tbwin;
};

typedef struct unit_area_s *unit_area_ptr;
typedef struct unit_area_s unit_area_t[1];

void unit_area_init(unit_area_ptr);
void unit_area_clear(unit_area_ptr ma);
unit_area_ptr unit_area_new();
void unit_area_edit(unit_area_ptr ma, machine_info_ptr mi);
void unit_area_center(unit_area_ptr ma, unit_ptr m);

#endif //UNIT_AREA_H
