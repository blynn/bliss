#ifndef SPREADSHEET_H
#define SPREADSHEET_H

#include "pattern.h"
#include "widget.h"
#include "grid.h"

struct spreadsheet_s {
    grid_t grid;
    pattern_ptr pattern;
};

typedef struct spreadsheet_s *spreadsheet_ptr;
typedef struct spreadsheet_s spreadsheet_t[1];

void spreadsheet_init(spreadsheet_ptr);
void spreadsheet_clear(spreadsheet_ptr);

spreadsheet_ptr spreadsheet_new();

#endif //SPREADSHEET_H
