#include "buzz_machine.h"
#include "machine.h"
#include "cell_area.h"
#include "spreadsheet.h"

static void cell_area_update(widget_ptr w)
{
    cell_area_ptr ca = (cell_area_ptr) w;
    cell_ptr c;
    machine_ptr m = ca->ss->pattern->machine;
    buzz_machine_info_ptr bmi;

    c = spreadsheet_current_cell(ca->ss);

    widget_draw_inverse_border(w);
    bmi = m->mi->buzzmi;
    if (bmi) {
	int i = spreadsheet_current_col(ca->ss);
	buzz_param_ptr p;
	if (i < bmi->gparam->count) {
	    p = (buzz_param_ptr) bmi->gparam->item[i];
	} else {
	    p = (buzz_param_ptr) bmi->tparam->item[(i - bmi->gparam->count) % bmi->tparam->count];
	}
	widget_write(w, 3, 3, p->desc);
    }
}

void cell_area_clear(cell_area_ptr ca)
{
    //TODO
}

void cell_area_init(cell_area_ptr ca, spreadsheet_ptr ss)
{
    widget_ptr w = ca->widget;
    widget_init(w);
    w->update = cell_area_update;
    ca->ss = ss;
}
