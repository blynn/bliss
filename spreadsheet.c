#include <stdlib.h>
#include "spreadsheet.h"
#include "song.h"
#include "util.h"

enum {
    cell_w = 64,
    cell_h = 16,
    rowlabel_w = 40,
    border = 1,
    rowlimit = 8192,
    collimit = 1024,
};

//TODO: change grid.c to utility routines.

static void spreadsheet_update(widget_ptr w)
{
    grid_ptr g = (grid_ptr) w;
    spreadsheet_ptr ss = (spreadsheet_ptr) w;
    pattern_ptr p = ss->pattern;
    track_ptr t = p->machine->track;
    song_ptr s = p->machine->song;

    grid_update(w);

    if (p == p->machine->track->play_pattern) {
	grid_draw_hline(g, s->tickcount - t->play_tick, 0, 1);
    }
}

static void spreadsheet_row_click(grid_ptr g, int r)
{
}

static char *spreadsheet_at(grid_ptr g, int r, int c)
{
    spreadsheet_ptr ss = (spreadsheet_ptr) g;
    if (!ss->pattern) return NULL;
    return pattern_at(ss->pattern, c, r);
}

static void spreadsheet_insert(grid_ptr g, int r, int c)
{
    spreadsheet_ptr ss = (spreadsheet_ptr) g;
    if (!ss->pattern) return;
    pattern_insert(ss->pattern, c, r);
}

static void spreadsheet_delete(grid_ptr g, int r, int c)
{
    spreadsheet_ptr ss = (spreadsheet_ptr) g;
    if (!ss->pattern) return;
    pattern_delete(ss->pattern, c, r);
}

static void spreadsheet_put(grid_ptr g, char *text, int r, int c)
{
    spreadsheet_ptr ss = (spreadsheet_ptr) g;
    if (!ss->pattern) return;
    //NULL or empty text is deleted
    if (!text || !*text) pattern_remove(ss->pattern, c, r);
    else pattern_put(ss->pattern, text, c, r);
}

void spreadsheet_edit(spreadsheet_ptr ss, pattern_ptr p)
{
    ss->pattern = p;
}

void spreadsheet_init(spreadsheet_ptr ss)
{
    grid_ptr g = (grid_ptr) ss;

    grid_init(g);
    spreadsheet_edit(ss, NULL);
    g->at = spreadsheet_at;
    g->insert = spreadsheet_insert;
    g->delete = spreadsheet_delete;
    g->put = spreadsheet_put;
    g->row_click = spreadsheet_row_click;
    g->flag = 0;
    ((widget_ptr) ss)->update = spreadsheet_update;
}

void spreadsheet_clear(spreadsheet_ptr ss)
{
    grid_ptr g = (grid_ptr) ss;

    grid_clear(g);
}

spreadsheet_ptr spreadsheet_new()
{
    spreadsheet_ptr r = (spreadsheet_ptr) malloc(sizeof(struct spreadsheet_s));
    spreadsheet_init(r);
    return r;
}

cell_ptr spreadsheet_current_cell(spreadsheet_ptr ss)
{
    grid_ptr g = (grid_ptr) ss;
    return pattern_cell_at(ss->pattern, g->cc + g->oc, g->cr + g->or);
}

int spreadsheet_current_col(spreadsheet_ptr ss)
{
    grid_ptr g = (grid_ptr) ss;
    return g->cc + g->oc;
}
