#ifndef GUI_H
#define GUI_H

#include "voice.h"
#include "ins.h"
#include "utable.h"

#include "aux.h"
#include "canvas.h"
#include "bliss.h"

enum {
    nav_voice,
    nav_ins,
    nav_orch,
};

extern int navtype;
extern void *navthing;
extern node_ptr navoutnode;

void gui_edit_orch();
void gui_edit_voice(voice_ptr voice);
void gui_edit_ins(ins_ptr ins);
void gui_back();

extern ins_ptr current_ins;
extern widget_t canvas;

edge_ptr add_edge(graph_ptr g, node_ptr src, node_ptr dst, int port);

#endif //GUI_H
