#ifndef UNIT_SIDEBAR_H
#define UNIT_SIDEBAR_H

#include "combobox.h"
#include "unit_area.h"

struct unit_sidebar_s {
    widget_t widget;
    unit_area_ptr ua;
    darray_t inlist;
    darray_t outlist;
    combobox_t cbinport;
    combobox_t cboutport;
    container_t con;
};

typedef struct unit_sidebar_s *unit_sidebar_ptr;
typedef struct unit_sidebar_s unit_sidebar_t[1];

void unit_sidebar_init(unit_sidebar_ptr ca, unit_area_ptr ua);
void unit_sidebar_clear(unit_sidebar_ptr ca);

#endif //UNIT_SIDEBAR_H
