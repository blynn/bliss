#ifndef CELL_AREA_H
#define CELL_AREA_H

#include "spreadsheet.h"

struct cell_area_s {
    widget_t widget;
    spreadsheet_ptr ss;
};

typedef struct cell_area_s *cell_area_ptr;
typedef struct cell_area_s cell_area_t[1];

void cell_area_init(cell_area_ptr ca, spreadsheet_ptr ss);
void cell_area_clear(cell_area_ptr ca);

#endif //CELL_AREA_H
