#include <stdlib.h>
#include <string.h>
#include "grid.h"
#include "song.h"
#include "util.h"
#include "textbox.h"

enum {
    cell_w = 50,
    cell_h = 16,
    rowlabel_w = 40,
    border = 1,
    rowlimit = 8192,
    collimit = 1024,
};

static int c2x(int c)
{
    return c * (cell_w + border) + rowlabel_w + border;
}

static int r2y(int r)
{
    return r * (cell_h + border) + cell_h + border;
}

static void grid_scroll(grid_ptr g, int x, int y)
{
    g->oc += x;
    g->or += y;
    if (g->oc < 0) g->oc = 0;
    if (g->or < 0) g->or = 0;
    if (g->oc + g->cmax >= collimit) {
	g->oc = collimit - g->cmax - 1;
    }
    if (g->or + g->rmax >= rowlimit) {
	g->or = rowlimit - g->rmax - 1;
    }
}

static void move_cursor(grid_ptr g, int x, int y)
{
    g->cc += x;
    if (g->cc < 0) {
	g->oc += g->cc;
	if (g->oc < 0) g->oc = 0;
	g->cc = 0;
    } else if (g->cc >= g->cmax) {
	g->oc += g->cc - g->cmax + 1;
	g->cc = g->cmax - 1;
	if (g->oc + g->cmax >= collimit) {
	    g->oc = collimit - g->cmax - 1;
	}
    }
    g->cr += y;
    if (g->cr < 0) {
	g->or += g->cr;
	if (g->or < 0) g->or = 0;
	g->cr = 0;
    } else if (g->cr >= g->rmax) {
	g->or += g->cr - g->rmax + 1;
	g->cr = g->rmax - 1;
	if (g->or + g->rmax >= rowlimit) {
	    g->or = rowlimit - g->rmax - 1;
	}
    }
}

inline static void move_cursor_up(grid_ptr g)
{
    move_cursor(g, 0, -1);
}

inline static void move_cursor_down(grid_ptr g)
{
    move_cursor(g, 0, 1);
}

inline static void insert_at_cursor(grid_ptr g)
{
    g->insert(g, g->cr + g->or, g->cc + g->oc);
}

inline static void delete_at_cursor(grid_ptr g)
{
    g->delete(g, g->cr + g->or, g->cc + g->oc);
}

static void text_put_cursor(grid_ptr g, char *text)
{
    g->put(g, text, g->cr + g->or, g->cc + g->oc);
}

static void edit_cursor(grid_ptr g)
{
    widget_put_local((widget_ptr) g->editbox, c2x(g->cc), r2y(g->cr));
    g->is_edit = 1;
    //textbox_put_text(g->editbox, text_at_cursor(g));
    textbox_put_text(g->editbox, NULL);
}

static void keep_changes(grid_ptr g)
{
    textbox_ptr b = g->editbox;
    text_put_cursor(g, b->text);
    g->is_edit = 0;
}

static void discard_changes(grid_ptr g)
{
    g->is_edit = 0;
}

static void handle_key(widget_ptr w, int key)
{
    grid_ptr g = (grid_ptr) w;
    if (g->is_edit) {
	switch (key) {
	    case SDLK_RETURN:
		keep_changes(g);
		move_cursor_down(g);
		break;
	    case SDLK_ESCAPE:
		discard_changes(g);
		break;
	    case SDLK_UP:
		keep_changes(g);
		move_cursor_up(g);
		break;
	    case SDLK_DOWN:
		keep_changes(g);
		move_cursor_down(g);
		break;
	    default:
		textbox_handle_key((widget_ptr) g->editbox, key);
		break;
	}
    } else {
	switch (key) {
	    case SDLK_INSERT:
		insert_at_cursor(g);
		break;
	    case SDLK_LEFT:
		move_cursor(g, -1, 0);
		break;
	    case SDLK_RIGHT:
		move_cursor(g, 1, 0);
		break;
	    case SDLK_UP:
		move_cursor_up(g);
		break;
	    case SDLK_DOWN:
		move_cursor_down(g);
		break;
	    case SDLK_DELETE:
		delete_at_cursor(g);
		break;
	    default:
		if (key < 32) break;
		if (key > 255) break;
		edit_cursor(g);
		textbox_handle_key((widget_ptr) g->editbox, key);
		break;
	}
    }
}

static int handle_left_click(widget_ptr w)
{
    int x, y;
    grid_ptr g = (grid_ptr) w;

    widget_getmousexy(w, &x, &y);
    if (x <= rowlabel_w && y > cell_h && y < g->ymax) {
	int r;
	r = y - cell_h - border;
	r /= cell_h + border;
	g->row_click(g, r + g->or);
    } else if (x > rowlabel_w && x < g->xmax && y > cell_h && y < g->ymax) {
	int r, c;
	c = x - rowlabel_w - border;
	c /= cell_w + border;
	r = y - cell_h - border;
	r /= cell_h + border;
	if (g->is_edit) keep_changes(g);
	g->cr = r;
	g->cc = c;
	return 1;
    }
    return 0;
}

int grid_handle_event(widget_ptr w, event_ptr e)
{
    grid_ptr g = (grid_ptr) w;
    switch (e->type) {
	case SDL_MOUSEBUTTONDOWN:
	    switch(e->button.button) {
		case 4:
		    grid_scroll(g, 0, -1);
		    break;
		case 5:
		    grid_scroll(g, 0, 1);
		    break;
		case SDL_BUTTON_LEFT:
		    handle_left_click(w);
		    break;
		default:
		    break;
	    }
	    break;
	case SDL_KEYDOWN:
	    handle_key(w, e->key.keysym.sym);
	    break;
    }
    return 1;
}

static void grid_sized(widget_ptr w, void *data)
{
    grid_ptr g = (grid_ptr) w;
    g->cmax = w->w - rowlabel_w - border;
    g->cmax /= (border + cell_w);
    g->rmax = w->h - cell_h - border;
    g->rmax /= (border + cell_h);
    g->ymax = cell_h + g->rmax * (cell_h + border);
    g->xmax = rowlabel_w + g->cmax * (cell_w + border);
    widget_put_size((widget_ptr) g->editbox, cell_w, cell_h);
}

static char *grid_row_label(grid_ptr g, int r)
{
    static char s[8];
    sprintf(s, "%04d", r);
    return s;
}

static char *grid_col_label(grid_ptr g, int c)
{
    static char s[8];
    sprintf(s, "%02d", c);
    return s;
}

static void grid_put(grid_ptr g, char *text, int r, int c)
{
}

static char *grid_at(grid_ptr g, int r, int c)
{
    return NULL;
}

void grid_update(widget_ptr w)
{
    grid_ptr g = (grid_ptr) w;
    int r, c;
    int x, y;
    SDL_Rect rect;

    rect.x = 0;
    rect.y = cell_h;
    rect.w = g->xmax;
    rect.h = border;
    widget_fillrect(w, &rect, c_text);
    x = 0;
    y = cell_h + border;
    for (r=0; r<g->rmax; r++) {
	char *s = g->row_label(g, r + g->or);
	if (s) {
	    widget_write(w, x + 4, y, s);
	}
	y += border + cell_h;
	rect.y += cell_h + border;
	if (!(g->flag & flag_hide_row_borders)) {
	    widget_fillrect(w, &rect, c_gridline);
	}
    }
    rect.x = rowlabel_w;
    rect.y = 0;
    rect.w = border;
    rect.h = g->ymax;
    widget_fillrect(w, &rect, c_text);
    x = rowlabel_w + border;
    y = 0;
    for (c=0; c<g->cmax; c++) {
	char *s = g->col_label(g, c + g->oc);
	if (s) {
	    widget_write(w, x + 4, y, s);
	}
	x += border + cell_w;
	rect.x += cell_w + border;
	widget_fillrect(w, &rect, c_text);
    }

    //draw cursor/editbox
    if (g->is_edit) {
	widget_update((widget_ptr) g->editbox);
    } else {
	x = c2x(g->cc);
	y = r2y(g->cr);
	rect.x = x;
	rect.y = y;
	rect.w = cell_w;
	rect.h = 2;
	widget_fillrect(w, &rect, c_cursor);
	rect.y = y + cell_h - 2;
	widget_fillrect(w, &rect, c_cursor);
	rect.y = y;
	rect.w = 2;
	rect.h = cell_h;
	widget_fillrect(w, &rect, c_cursor);
	rect.x = x + cell_w - 2;
	widget_fillrect(w, &rect, c_cursor);
    }

    //draw spreadsheet
    for (r=0; r<g->rmax; r++) {
	for (c=0; c<g->cmax; c++) {
	    char *s;
	    //avoid box under cursor if editing
	    if (r == g->cr && c == g->cc && 
		    g->is_edit) continue;
	    s = g->at(g, r + g->or, c + g->oc);
	    if (s) {
		x = c2x(c);
		y = r2y(r);
		widget_write(w, x, y, s);
	    }
	}
    }
}

void grid_draw_hline(grid_ptr g, int row, int p, int q)
{
    widget_ptr w = (widget_ptr) g;
    SDL_Rect r;
    r.y = row - g->or;
    if (r.y < 0) return;
    r.y = r2y(r.y);
    r.y += cell_h * p / q;
    r.x = 0;
    r.w = w->w;
    r.h = 1;
    if (r.y < g->ymax) widget_fillrect(w, &r, c_border);
}

void grid_init(grid_ptr g)
{
    widget_init(g->widget);
    g->widget->handle_event = grid_handle_event;
    widget_connect(g->widget, signal_resize, grid_sized, NULL);
    g->widget->update = grid_update;
    g->cc = 0;
    g->cr = 0;
    g->or = 0;
    g->oc = 0;
    g->row_label = grid_row_label;
    g->col_label = grid_col_label;
    g->at = grid_at;
    g->put = grid_put;
    textbox_init(g->editbox);
    g->editbox->appear_active = 1;
    g->is_edit = 0;
    ((widget_ptr) g->editbox)->parent = (widget_ptr) g;
}

void grid_clear(grid_ptr g)
{
    textbox_clear(g->editbox);
    widget_clear(g->widget);
}

grid_ptr grid_new()
{
    grid_ptr r = (grid_ptr) malloc(sizeof(struct grid_s));
    grid_init(r);
    return r;
}
