#include <stdlib.h>
#include "pattern_area.h"

enum {
    padding = 5,
    header_h = 16,
    button_w = 13,
    button_h = 13,
};

void pattern_area_sized(widget_ptr w, void *data)
{
    pattern_area_ptr p = (pattern_area_ptr) w;

    widget_put_size((widget_ptr) p->con, w->w, w->h);
    widget_put_size((widget_ptr) p->ss, w->w, w->h - header_h - padding);
}

void pattern_area_moved(widget_ptr w, void *data)
{
    pattern_area_ptr p = (pattern_area_ptr) w;

    widget_notify_move((widget_ptr) p->con);
}

void pattern_area_update(widget_ptr w)
{
    pattern_area_ptr p = (pattern_area_ptr) w;
    widget_update((widget_ptr) p->con);
}

int pattern_area_handle_event(widget_ptr w, event_ptr e)
{
    pattern_area_ptr p = (pattern_area_ptr) w;

    return widget_handle_event((widget_ptr) p->con, e);
}

static void workout_pattern_list(pattern_area_ptr pa)
{
    darray_ptr a;
    int i, n;

    a = pa->machine->pattern;

    darray_remove_all(pa->pattern_list);

    n = a->count;
    for (i=0; i<n; i++) {
	pattern_ptr p = (pattern_ptr) a->item[i];
	darray_append(pa->pattern_list, p->id);
    }
}

void pattern_area_put_machine(pattern_area_t pa, machine_ptr m)
{
    pa->machine = m;
    listbox_put_text(pa->lbmachine, pa->machine->id);
    if (m->pattern->count) {
	pattern_area_put_pattern(pa, (pattern_ptr) m->pattern->item[0]);
    } else {
	pattern_area_put_pattern(pa, NULL);
    }
    workout_pattern_list(pa);
}

void pattern_area_put_pattern(pattern_area_t pa, pattern_ptr p)
{
    pa->pattern = p;
    if (p) {
	pa->machine = p->machine;
	listbox_put_text(pa->lbmachine, pa->machine->id);
	pa->ss->pattern = p;
	widget_show((widget_ptr) pa->ss);
	listbox_put_text(pa->lbpattern, p->id);
    } else {
	widget_hide((widget_ptr) pa->ss);
	listbox_put_text(pa->lbpattern, "--none--");
    }
    workout_pattern_list(pa);
}

void pattern_area_edit(pattern_area_t pa, song_ptr song)
{
    pa->song = song;
    pattern_area_put_machine(pa, song->master);
    pa->lbmachine->choice = song->mid_list;
}

SDL_Surface *make_arrow(int dir)
{
    Sint16 xtri[3], ytri[3];
    SDL_Surface *img;
    img = new_image(13, 13);
    ytri[0] = 6;
    ytri[1] = 2;
    ytri[2] = 10;
    if (dir < 0) {
	xtri[0] = 2;
	xtri[1] = 10;
	xtri[2] = 10;
    } else {
	xtri[0] = 10;
	xtri[1] = 2;
	xtri[2] = 2;
    }
    SDL_FillRect(img, NULL, colour[c_background]);
    filledPolygonColor(img, xtri, ytri, 3, colourgfx(c_text));
    return img;
}

static void prev_mach(widget_ptr caller, void *data)
{
    widget_ptr w = (widget_ptr) data;
    pattern_area_ptr p = (pattern_area_ptr) w;
    darray_ptr a = p->song->machine;
    int i = darray_index_of(a, p->machine);
    i--;
    if (i < 0) i = a->count - 1;
    pattern_area_put_machine(p, (machine_ptr) a->item[i]);
}

static void next_mach(widget_ptr caller, void *data)
{
    widget_ptr w = (widget_ptr) data;
    pattern_area_ptr p = (pattern_area_ptr) w;
    darray_ptr a = p->song->machine;
    int i = darray_index_of(a, p->machine);
    i++;
    if (i >= a->count) i = 0;
    pattern_area_put_machine(p, (machine_ptr) a->item[i]);
}

static void prev_pat(widget_ptr caller, void *data)
{
    widget_ptr w = (widget_ptr) data;
    pattern_area_ptr p = (pattern_area_ptr) w;
    darray_ptr a = p->machine->pattern;
    int i;
    if (!p->pattern) return;
    i = darray_index_of(a, p->pattern);
    i--;
    if (i < 0) i = a->count - 1;
    pattern_area_put_pattern(p, (pattern_ptr) a->item[i]);
}

static void next_pat(widget_ptr caller, void *data)
{
    widget_ptr w = (widget_ptr) data;
    pattern_area_ptr p = (pattern_area_ptr) w;
    darray_ptr a = p->machine->pattern;
    int i;
    if (!p->pattern) return;
    i = darray_index_of(a, p->pattern);
    i++;
    if (i >= a->count) i = 0;
    pattern_area_put_pattern(p, (pattern_ptr) a->item[i]);
}

static void new_pat_cb(widget_ptr caller, void *data)
{
    widget_ptr w = (widget_ptr) data;
    pattern_area_ptr pa = (pattern_area_ptr) w;
    pattern_ptr p;
    p = machine_create_pattern_auto_id(pa->machine);
    root_edit_pattern(p);
}

static void del_pat_cb(widget_ptr caller, void *data)
{
    widget_ptr w = (widget_ptr) data;
    pattern_area_ptr pa = (pattern_area_ptr) w;
    machine_ptr m = pa->machine;
    int i, n;
    n = m->pattern->count;
    for (i=0; i<n; i++) {
	pattern_ptr p = (pattern_ptr) m->pattern->item[i];
	if (p == pa->pattern) {
	    darray_remove(m->pattern, p);
	    pattern_clear(p);
	    free(p);
	    if (i >= n - 1) i--;
	    if (i < 0) p = NULL;
	    else p = (pattern_ptr) m->pattern->item[i];
	    root_edit_pattern(p);
	    break;
	}
    }
}

static void put_machine_cb(widget_ptr w, void *data)
{
    listbox_ptr b = (listbox_ptr) w;
    pattern_area_ptr pa = (pattern_area_ptr) data;
    machine_ptr m = song_machine_at(pa->song, b->text);
    pattern_area_put_machine(pa, m);
}

static void put_pattern_cb(widget_ptr w, void *data)
{
    listbox_ptr b = (listbox_ptr) w;
    pattern_area_ptr pa = (pattern_area_ptr) data;
    pattern_ptr p = machine_pattern_at(pa->machine, b->text);
    pattern_area_put_pattern(pa, p);
}

void pattern_area_init(pattern_area_t p)
{
    widget_ptr w = (widget_ptr) p;
    widget_ptr w1;

    //TODO: put this somewhere global
    SDL_Surface *image_left_arrow = make_arrow(-1);
    SDL_Surface *image_right_arrow = make_arrow(1);
    SDL_Surface *image_newpattern;
    SDL_Surface *image_deletepattern;

    widget_init(w);
    container_init(p->con);
    spreadsheet_init(p->ss);
    button_init(p->bmback);
    button_init(p->bmforward);
    button_init(p->bpback);
    button_init(p->bpforward);
    button_init(p->bpnew);
    button_init(p->bpdelete);

    w->update = pattern_area_update;
    w->handle_event = pattern_area_handle_event;
    widget_connect(w, signal_resize, pattern_area_sized, NULL);
    widget_connect(w, signal_move, pattern_area_moved, NULL);

    button_put_image(p->bmback, image_left_arrow);
    button_shrinkwrap(p->bmback);
    widget_connect((widget_ptr) p->bmback, signal_activate, prev_mach, w);

    button_put_image(p->bmforward, image_right_arrow);
    button_shrinkwrap(p->bmforward);
    widget_connect((widget_ptr) p->bmforward, signal_activate, next_mach, w);

    button_put_image(p->bpback, image_left_arrow);
    button_shrinkwrap(p->bpback);
    widget_connect((widget_ptr) p->bpback, signal_activate, prev_pat, w);

    button_put_image(p->bpforward, image_right_arrow);
    button_shrinkwrap(p->bpforward);
    widget_connect((widget_ptr) p->bpforward, signal_activate, next_pat, w);

    button_put_image(p->bpforward, image_right_arrow);
    button_shrinkwrap(p->bpforward);
    widget_connect((widget_ptr) p->bpforward, signal_activate, next_pat, w);

    image_newpattern = font_rendertext("New");
    button_put_image(p->bpnew, image_newpattern);
    button_shrinkwrap(p->bpnew);
    widget_connect((widget_ptr) p->bpnew, signal_activate, new_pat_cb, w);

    image_deletepattern = font_rendertext("Delete");
    button_put_image(p->bpdelete, image_deletepattern);
    button_shrinkwrap(p->bpdelete);
    widget_connect((widget_ptr) p->bpdelete, signal_activate, del_pat_cb, w);

    listbox_init(p->lbmachine);
    widget_put_size((widget_ptr) p->lbmachine, 100 - 15, 18);
    widget_connect((widget_ptr) p->lbmachine, signal_activate, put_machine_cb, w);

    listbox_init(p->lbpattern);
    widget_put_size((widget_ptr) p->lbpattern, 100 - 15, 18);
    p->lbpattern->choice = p->pattern_list;
    widget_connect((widget_ptr) p->lbpattern, signal_activate, put_pattern_cb, w);

    container_put_widget(p->con, (widget_ptr) p->bmback, 0, 0);
    container_put_widget(p->con, (widget_ptr) p->lbmachine, 22, 0);
    container_put_widget(p->con, (widget_ptr) p->bmforward, 110, 0);
    container_put_widget(p->con, (widget_ptr) p->bpback, 200, 0);
    container_put_widget(p->con, (widget_ptr) p->lbpattern, 222, 0);
    container_put_widget(p->con, (widget_ptr) p->bpforward, 310, 0);
    container_put_widget(p->con, (widget_ptr) p->bpnew, 340, 0);
    container_put_widget(p->con, (widget_ptr) p->bpdelete, 370, 0);
    container_put_widget(p->con, (widget_ptr) p->ss, 0, header_h + padding);
    w1 = (widget_ptr) p->con;
    w1->parent = w;

    darray_init(p->pattern_list);
}

void pattern_area_clear(pattern_area_t p)
{
    container_clear(p->con);
    spreadsheet_clear(p->ss);
    button_clear(p->bmback);
    button_clear(p->bmforward);
    button_clear(p->bpback);
    button_clear(p->bpforward);
    listbox_clear(p->lbmachine);
    listbox_clear(p->lbpattern);
    widget_clear((widget_ptr) p);

    darray_clear(p->pattern_list);
}

pattern_area_ptr pattern_area_new()
{
    pattern_area_ptr p;
    p = (pattern_area_ptr) malloc(sizeof(struct pattern_area_s));
    pattern_area_init(p);
    return p;
}
