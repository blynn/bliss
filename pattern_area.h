#ifndef PATTERN_AREA_H
#define PATTERN_AREA_H

#include "container.h"
#include "spreadsheet.h"
#include "machine.h"
#include "button.h"
#include "listbox.h"
#include "song.h"

struct pattern_area_s {
    widget_t widget;
    container_t con;
    spreadsheet_t ss;
    song_ptr song;
    machine_ptr machine;
    pattern_ptr pattern;
    listbox_t lbmachine;
    listbox_t lbpattern;
    button_t bmback;
    button_t bmforward;
    button_t bpback;
    button_t bpforward;
};

typedef struct pattern_area_s *pattern_area_ptr;
typedef struct pattern_area_s pattern_area_t[1];

void pattern_area_init(pattern_area_ptr);
void pattern_area_clear(pattern_area_ptr);
pattern_area_ptr pattern_area_new();
void pattern_area_put_machine(pattern_area_t pa, machine_ptr m);
void pattern_area_put_pattern(pattern_area_t pa, pattern_ptr m);
void pattern_area_edit(pattern_area_t pa, song_ptr p);

#endif //PATTERN_AREA_H
