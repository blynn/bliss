#include "SDL_gfxPrimitives.h"

#include "widget.h"
#include "colour.h"

static SDL_Surface *screen;

void widget_set_screen(SDL_Surface *s)
{
    screen = s;
}

int widget_contains(widget_ptr r, int x, int y)
{
    if (x < r->x) return 0;
    if (y < r->y) return 0;
    if (x >= r->x + r->w) return 0;
    if (y >= r->y + r->h) return 0;
    return -1;
}

void widget_shift_rect(widget_ptr r, widget_ptr shift)
{
    r->x += shift->x;
    r->y += shift->y;
}

void widget_line(widget_ptr r, int x0, int y0, int x1, int y1, int c)
{
    lineColor(screen, r->x + x0, r->y + y0, r->x + x1, r->y + y1, colourgfx[c]);
}

void widget_box(widget_ptr r, int x0, int y0, int x1, int y1, int c)
{
    boxColor(screen, r->x + x0, r->y + y0, r->x + x1, r->y + y1, colourgfx[c]);
}

void image_box_rect(image_ptr img, int c)
{
    boxColor(img, 0, 0, img->w - 1, img->h - 1, colourgfx[c]);
}

void widget_box_rect(widget_ptr r, int c)
{
    boxColor(screen, r->x, r->y, r->x + r->w - 1, r->y + r->h - 1, colourgfx[c]);
}

void widget_rectangle(widget_ptr r, int x0, int y0, int x1, int y1, int c)
{
    rectangleColor(screen, r->x + x0, r->y + y0, r->x + x1, r->y + y1, colourgfx[c]);
}

void widget_rectangle_rect(widget_ptr r, int c)
{
    rectangleColor(screen, r->x, r->y,
	    r->x + r->w - 1, r->y + r->h - 1, colourgfx[c]);
}

void widget_string(widget_ptr r, int x, int y, char *s, int c)
{
    stringColor(screen, r->x + x, r->y + y, s, colourgfx[c]);
}

void image_string(image_ptr p, int x, int y, char *s, int c)
{
    stringColor(p, x, y, s, colourgfx[c]);
}

void widget_raised_border(widget_ptr rect)
{
    int x0, y0;
    x0 = rect->w - 1;
    y0 = rect->h - 1;

    widget_box(rect, 1, 1, x0 - 1, 1, c_background);
    widget_box(rect, 1, 1, 1, y0 - 1, c_background);

    widget_box(rect, 0, 0, x0, 0, c_highlight);
    widget_box(rect, 0, 0, 0, y0, c_highlight);

    widget_box(rect, 1, y0 - 1, x0 - 1, y0 - 1, c_shadow);
    widget_box(rect, x0 - 1, 1, x0 - 1, y0 - 1, c_shadow);

    widget_box(rect, 0, y0, x0, y0, c_darkshadow);
    widget_box(rect, x0, 0, x0, y0, c_darkshadow);
}

void widget_raised_background(widget_ptr rect)
{
    widget_box_rect(rect, c_background);
    widget_raised_border(rect);
}

void widget_lowered_border(widget_ptr rect)
{
    int x1, y1;
    x1 = rect->w - 1;
    y1 = rect->h - 2;
    widget_box(rect, 2, y1, x1 - 1, y1, c_background);
    widget_box(rect, x1 - 1, 2, x1 - 1, y1, c_background);
    y1++;
    widget_box(rect, 1, y1, x1 - 1, y1, c_highlight);
    widget_box(rect, x1, 1, x1, y1, c_highlight);

    widget_box(rect, 0, 0, 0, y1, c_shadow);
    widget_box(rect, 0, 0, x1, 0, c_shadow);

    widget_box(rect, 1, 1, 1, y1 - 1, c_darkshadow);
    widget_box(rect, 1, 1, x1 - 1, 1, c_darkshadow);
}

void widget_filled_circle(widget_ptr w, int x, int y, int r, int c)
{
    filledCircleColor(screen, w->x + x, w->y + y, r, colourgfx[c]);
}

void widget_blit(widget_ptr rect, int x, int y, image_ptr img)
{
    SDL_Rect r;

    r.x = rect->x + x;
    r.y = rect->y + y;
    SDL_BlitSurface(img, NULL, screen, &r);
}

void widget_translate(widget_ptr wid, int x, int y)
{
    wid->localx += x;
    wid->localy += y;
    wid->x += x;
    wid->y += y;
}

void widget_clip(widget_ptr wid)
{
    rect_t r;
    r->x = wid->x;
    r->y = wid->y;
    r->w = wid->w;
    r->h = wid->h;
    SDL_SetClipRect(screen, r);
}

void widget_unclip()
{
    SDL_SetClipRect(screen, NULL);
}

void widget_move_children(widget_ptr w)
{
    int i;
    for (i=0; i<w->child->count; i++) {
	widget_ptr child = (widget_ptr) w->child->item[i];
	child->x = w->x + child->localx;
	child->y = w->y + child->localy;
	widget_move_children(child);
    }
}

void widget_handle_keydown(widget_ptr w, int sym, int mod)
{
}

void widget_handle_mousebuttondown(widget_ptr w, int button, int x, int y)
{
    int i;
    for (i=0; i<w->show_list->count; i++) {
	widget_ptr w1 = (widget_ptr) w->show_list->item[i];
	if (widget_contains(w1, x, y)) {
	    w1->handle_mousebuttondown(w1, button, x, y);
	    return;
	}
    }
}

void widget_handle_mousebuttonup(widget_ptr w, int button, int x, int y)
{
}

void widget_show(widget_ptr w)
{
    darray_append(w->parent->show_list, w);
    w->x = w->x + w->parent->localx;
    w->y = w->y + w->parent->localy;
}

void widget_hide(widget_ptr w)
{
    darray_remove(w->parent->show_list, w);
}

void default_widget_update(widget_ptr w)
{
    int i;

    for (i=0; i<w->show_list->count; i++) {
	widget_ptr p = w->show_list->item[i];
	p->update(p);
    }
}

void widget_put_size(widget_ptr w, int x, int y)
{
    w->w = x;
    w->h = y;
}

void widget_init(widget_ptr w, widget_ptr parent)
{
    darray_init(w->child);
    darray_init(w->show_list);
    w->parent = parent;
    darray_append(parent->child, w);
    w->handle_mousebuttonup = widget_handle_mousebuttonup;
    w->handle_mousebuttondown = widget_handle_mousebuttondown;
    w->handle_keydown = widget_handle_keydown;
    w->update = default_widget_update;
    w->put_size = widget_put_size;
}

image_ptr image_new(int w, int h)
{
    return SDL_CreateRGBSurface(SDL_HWSURFACE,
	    w, h, 32,
	    screen->format->Rmask,
	    screen->format->Gmask,
	    screen->format->Bmask,
	    screen->format->Amask);
}

void image_clear(image_ptr img)
{
    SDL_FreeSurface(img);
}
