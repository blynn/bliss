#ifndef MACHINE_AREA_H
#define MACHINE_AREA_H

#include "machine.h"
#include "widget.h"
#include "menu.h"
#include "song.h"
#include "tbwin.h"

enum {
    m_w = 80,
    m_h = 40
};

struct machine_area_s {
    widget_t widget;
    int drag_flag;
    int drag_startx;
    int drag_starty;
    song_ptr song;
    machine_ptr drag_machine;
    edge_ptr sel_edge;
    menu_t machmenu;
    menu_t edgemenu;
    menu_t rootmenu;
    menu_t listmenu;
    darray_t zorder;
    tbwin_t tbwin;
};

typedef struct machine_area_s *machine_area_ptr;
typedef struct machine_area_s machine_area_t[1];

void machine_area_init(machine_area_ptr);
void machine_area_clear(machine_area_ptr ma);
machine_area_ptr machine_area_new();
void machine_area_edit(machine_area_ptr ma, song_ptr song);

#endif //MACHINE_AREA_H
