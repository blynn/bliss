#include <stdlib.h>
#include "song_area.h"
#include "menu.h"
#include "track.h"
#include "util.h"

static char *song_area_row_label(grid_ptr g, int r)
{
    static char s[8];
    song_area_ptr sa = (song_area_ptr) g;
    sprintf(s, "%04d", r * sa->step);
    return s;
}

static char *song_area_col_label(grid_ptr g, int c)
{
    song_area_ptr sa = (song_area_ptr) g;
    darray_ptr a = sa->song->machine;
    if (c >= a->count) return NULL;
    return ((machine_ptr) a->item[c])->id;
}

static char *song_area_at(grid_ptr g, int r, int c)
{
    char *s;
    song_area_ptr sa = (song_area_ptr) g;
    darray_ptr a = sa->song->machine;
    int row = r * sa->step;
    if (c >= a->count) return NULL;
    s = track_at(((machine_ptr) a->item[c])->track, row);
    if (s) return strclone(s);
    return NULL;
}

static void song_area_put(grid_ptr g, char *text, int r, int c)
{
    song_area_ptr sa = (song_area_ptr) g;
    darray_ptr a = sa->song->machine;
    track_ptr t;
    int row = r * sa->step;
    if (c >= a->count) return;
    //if (!text) pattern_delete(ss->pattern, c, row);
    //else pattern_put(ss->pattern, text, c, row);
    t = ((machine_ptr) a->item[c])->track;
    //text = empty or NULL is a delete
    if (!text || !*text) track_delete(t, row);
    else track_put(t, text, row);
}

void song_area_init(song_area_ptr sa)
{
    grid_ptr g = (grid_ptr) sa;

    grid_init(g);
    g->row_label = song_area_row_label;
    g->col_label = song_area_col_label;
    g->at = song_area_at;
    g->put = song_area_put;
    g->flag |= flag_hide_row_borders;
    sa->step = 16;
}

song_area_ptr song_area_new()
{
    song_area_ptr r = (song_area_ptr) malloc(sizeof(struct song_area_s));
    song_area_init(r);
    return r;
}

void song_area_edit(song_area_ptr sa, song_ptr song)
{
    sa->song = song;
}
