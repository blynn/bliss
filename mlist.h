#ifndef MLIST_H
#define MLIST_H

#include "machine.h"
#include "darray.h"

//machine list

//TODO: will eventually have tree structure
//that allows easy browsing of machines
//for now it's a list
extern darray_t mlist;

void load_plugin_dir(char *dirname);
machine_info_ptr machine_info_at(char *id);

#endif //MLIST_H
