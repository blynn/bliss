#ifndef MACHINE_AREA_H
#define MACHINE_AREA_H

#include "machine.h"
#include "widget.h"
#include "menu.h"
#include "song.h"
#include "tbwin.h"

enum {
    m_w = 64,
    m_h = 32
};

enum {
    cc_list,
    cc_node,
};

typedef void (*cc_callback_f)(void *);

struct context_command_s {
    int hotkey;
    char *name;
    char *desc;
    int type;
    darray_t list;
    cc_callback_f func;
    void *data;
};
typedef struct context_command_s *context_command_ptr;
typedef struct context_command_s context_command_t[1];

struct machine_area_s {
    widget_t widget;
    int drag_flag;
    int drag_startx;
    int drag_starty;
    song_ptr song;
    machine_ptr sel_machine;
    edge_ptr sel_edge;
    darray_t zorder;
    tbwin_t tbwin;
    menu_t menu;
    darray_t menu_list;
    darray_t menuitem_list;
    context_command_t cc_new_machine;
    context_command_t cc_rename, cc_delete, cc_new_pattern, cc_disconnect;
    context_command_t cc_edge, cc_machine, cc_song, cc_master;
    context_command_ptr cc_current;
};

typedef struct machine_area_s *machine_area_ptr;
typedef struct machine_area_s machine_area_t[1];

void machine_area_init(machine_area_ptr);
void machine_area_clear(machine_area_ptr ma);
machine_area_ptr machine_area_new();
void machine_area_edit(machine_area_ptr ma, song_ptr song);
void machine_area_center(machine_area_ptr ma, machine_ptr m);

#endif //MACHINE_AREA_H
