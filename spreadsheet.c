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

static char *spreadsheet_at(grid_ptr g, int r, int c)
{
    spreadsheet_ptr ss = (spreadsheet_ptr) g;
    if (!ss->pattern) return NULL;
    return pattern_at(ss->pattern, c, r);
}

static void spreadsheet_put(grid_ptr g, char *text, int r, int c)
{
    spreadsheet_ptr ss = (spreadsheet_ptr) g;
    if (!ss->pattern) return;
    //NULL or empty text is deleted
    if (!text || !*text) pattern_delete(ss->pattern, c, r);
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
    g->put = spreadsheet_put;
    g->flag = 0;
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
