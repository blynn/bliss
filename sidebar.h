#ifndef SIDEBAR_H
#define SIDEBAR_H

#include "machine.h"
#include "widget.h"
#include "container.h"
#include "machine_area.h"
#include "sidebar.h"

struct sidebar_s {
    widget_t widget;
    machine_area_ptr ma;
};

typedef struct sidebar_s *sidebar_ptr;
typedef struct sidebar_s sidebar_t[1];

void sidebar_init(sidebar_ptr, machine_area_ptr ma);
void sidebar_clear(sidebar_ptr ma);
sidebar_ptr sidebar_new();

#endif //SIDEBAR_H
