#ifndef MLIST_H
#define MLIST_H

#include "machine.h"
#include "unit.h"
#include "darray.h"

//machine list

//TODO: will eventually have tree structure
//that allows easy browsing of machines
//for now it's a list
extern darray_t mlist;
extern darray_t ulist;

extern machine_info_ptr master_mi;

void mlist_init();
void load_plugin_dir(char *dirname);
machine_info_ptr machine_info_at(char *id);
unit_info_ptr unit_info_at(char *id);

#endif //MLIST_H
