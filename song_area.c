#include <stdlib.h>
#include "song_area.h"
#include "menu.h"
#include "track.h"
#include "util.h"

static void song_area_update(widget_ptr w)
{
    song_area_ptr sa = (song_area_ptr) w;
    grid_ptr g = (grid_ptr) w;
    song_ptr s = sa->song;

    grid_update(w);

    grid_draw_hline(g, s->tickcount / sa->step, s->tickcount % sa->step, sa->step);
    grid_draw_hline(g, s->song_end / sa->step, 0, 1);
}

int song_area_handle_event(widget_ptr w, event_ptr e)
{
    song_area_ptr sa = (song_area_ptr) w;
    grid_ptr g = (grid_ptr) w;
    switch (e->type) {
	case SDL_KEYDOWN:
	    if (e->key.keysym.sym == SDLK_e && (widget_getmod(w) & KMOD_CTRL)) {
		sa->song->song_end = (g->cr + g->or) * sa->step;
		return 1;
	    }
    }
    return grid_handle_event(w, e);
}

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

static void song_area_row_click(grid_ptr g, int r)
{
    song_area_ptr sa = (song_area_ptr) g;
    int row = r * sa->step;
    song_jump_to_tick(sa->song, row);
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

static void song_area_insert(grid_ptr g, int r, int c)
{
    song_area_ptr sa = (song_area_ptr) g;
    darray_ptr a = sa->song->machine;
    track_ptr t;
    int row = r * sa->step;
    if (c >= a->count) return;
    t = ((machine_ptr) a->item[c])->track;
    track_insert(t, row, sa->step);
}

static void song_area_delete(grid_ptr g, int r, int c)
{
    song_area_ptr sa = (song_area_ptr) g;
    darray_ptr a = sa->song->machine;
    track_ptr t;
    int row = r * sa->step;
    if (c >= a->count) return;
    t = ((machine_ptr) a->item[c])->track;
    track_delete(t, row, sa->step);
}

static void song_area_put(grid_ptr g, char *text, int r, int c)
{
    song_area_ptr sa = (song_area_ptr) g;
    darray_ptr a = sa->song->machine;
    track_ptr t;
    int row = r * sa->step;
    if (c >= a->count) return;
    t = ((machine_ptr) a->item[c])->track;
    //text = empty or NULL is a remove
    if (!text || !*text) track_remove(t, row);
    else track_put(t, text, row);
}

void song_area_init(song_area_ptr sa)
{
    widget_ptr w = (widget_ptr) sa;
    grid_ptr g = (grid_ptr) sa;

    grid_init(g);
    g->row_label = song_area_row_label;
    g->col_label = song_area_col_label;
    g->at = song_area_at;
    g->insert = song_area_insert;
    g->delete = song_area_delete;
    g->put = song_area_put;
    g->row_click = song_area_row_click;
    g->flag |= flag_hide_row_borders;
    sa->step = 16;
    w->update = song_area_update;
    w->handle_event = song_area_handle_event;
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
