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

void pattern_area_moved(widget_ptr w)
{
    pattern_area_ptr p = (pattern_area_ptr) w;

    widget_moved(w);
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

void pattern_area_put_machine(pattern_area_t pa, machine_ptr m)
{
    pa->machine = m;
    listbox_put_text(pa->lbmachine, pa->machine->id);
    if (m->pattern->count) {
	pattern_area_put_pattern(pa, (pattern_ptr) m->pattern->item[0]);
    } else {
	pattern_area_put_pattern(pa, NULL);
    }
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
}

void pattern_area_edit(pattern_area_t pa, song_ptr song)
{
    pa->song = song;
    pattern_area_put_machine(pa, song->master);
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

void pattern_area_init(pattern_area_t p)
{
    widget_ptr w = (widget_ptr) p;
    widget_ptr w1;

    //TODO: put this somewhere global
    SDL_Surface *image_left_arrow = make_arrow(-1);
    SDL_Surface *image_right_arrow = make_arrow(1);

    widget_init(w);
    container_init(p->con);
    spreadsheet_init(p->ss);
    button_init(p->bmback);
    button_init(p->bmforward);
    button_init(p->bpback);
    button_init(p->bpforward);

    w->update = pattern_area_update;
    w->handle_event = pattern_area_handle_event;
    widget_connect(w, signal_resize, pattern_area_sized, NULL);
    w->moved = pattern_area_moved;

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

    listbox_init(p->lbmachine);
    widget_put_size((widget_ptr) p->lbmachine, 100 - 17, 15);

    listbox_init(p->lbpattern);
    widget_put_size((widget_ptr) p->lbpattern, 100 - 17, 15);

    container_put_widget(p->con, (widget_ptr) p->bmback, 0, 0);
    container_put_widget(p->con, (widget_ptr) p->lbmachine, 16, 0);
    container_put_widget(p->con, (widget_ptr) p->lbpattern, 216, 0);
    container_put_widget(p->con, (widget_ptr) p->bmforward, 100, 0);
    container_put_widget(p->con, (widget_ptr) p->bpback, 200, 0);
    container_put_widget(p->con, (widget_ptr) p->bpforward, 300, 0);
    container_put_widget(p->con, (widget_ptr) p->ss, 0, header_h + padding);
    w1 = (widget_ptr) p->con;
    w1->parent = w;
    pattern_area_put_pattern(p, NULL);
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
}

pattern_area_ptr pattern_area_new()
{
    pattern_area_ptr p;
    p = (pattern_area_ptr) malloc(sizeof(struct pattern_area_s));
    pattern_area_init(p);
    return p;
}
