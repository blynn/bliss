#include "widget.h"
#include "colour.h"

#include "SDL_gfxPrimitives.h"

static SDL_Surface *screen;

void widget_set_screen(SDL_Surface *s)
{
    screen = s;
}

void widget_pixel(widget_ptr r, int x, int y, int c)
{
    pixelColor(screen, r->globalx + x, r->globaly + y, colourgfx[c]);
}

void widget_line(widget_ptr r, int x0, int y0, int x1, int y1, int c)
{
    lineColor(screen, r->globalx + x0, r->globaly + y0, r->globalx + x1, r->globaly + y1, colourgfx[c]);
}

void widget_filled_trigon(widget_ptr w,
	int x0, int y0,
	int x1, int y1,
	int x2, int y2,
	int c)
{
    filledTrigonColor(screen,
	    w->globalx + x0, w->globaly + y0,
	    w->globalx + x1, w->globaly + y1,
	    w->globalx + x2, w->globaly + y2,
	    colourgfx[c]);
}

void widget_box(widget_ptr r, int x0, int y0, int x1, int y1, int c)
{
    boxColor(screen, r->globalx + x0, r->globaly + y0, r->globalx + x1, r->globaly + y1, colourgfx[c]);
}

void image_box_rect(image_ptr img, int c)
{
    boxColor(img, 0, 0, img->w - 1, img->h - 1, colourgfx[c]);
}

void widget_box_rect(widget_ptr r, int c)
{
    boxColor(screen, r->globalx, r->globaly, r->globalx + r->w - 1, r->globaly + r->h - 1, colourgfx[c]);
}

void widget_rectangle(widget_ptr r, int x0, int y0, int x1, int y1, int c)
{
    rectangleColor(screen, r->globalx + x0, r->globaly + y0, r->globalx + x1, r->globaly + y1, colourgfx[c]);
}

void widget_rectangle_rect(widget_ptr r, int c)
{
    rectangleColor(screen, r->globalx, r->globaly,
	    r->globalx + r->w - 1, r->globaly + r->h - 1, colourgfx[c]);
}

void widget_string(widget_ptr r, int x, int y, char *s, int c)
{
    stringColor(screen, r->globalx + x, r->globaly + y, s, colourgfx[c]);
}

void image_string(image_ptr p, int x, int y, char *s, int c)
{
    stringColor(p, x, y, s, colourgfx[c]);
}


int widget_contains(widget_ptr w, int x, int y)
{
    if (x < 0) return 0;
    if (y < 0) return 0;
    if (x >= w->w) return 0;
    if (y >= w->h) return 0;
    return -1;
}

int local_contains(widget_ptr w, int x, int y)
{
    if (x < w->localx) return 0;
    if (y < w->localy) return 0;
    if (x >= w->localx + w->w) return 0;
    if (y >= w->localy + w->h) return 0;
    return -1;
}

int global_contains(widget_ptr r, int x, int y)
{
    if (x < r->globalx) return 0;
    if (y < r->globaly) return 0;
    if (x >= r->globalx + r->w) return 0;
    if (y >= r->globaly + r->h) return 0;
    return -1;
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

void image_filled_circle(image_ptr img, int x, int y, int r, int c)
{
    filledCircleColor(img, x, y, r, colourgfx[c]);
}

void widget_filled_circle(widget_ptr w, int x, int y, int r, int c)
{
    filledCircleColor(screen, w->globalx + x, w->globaly + y, r, colourgfx[c]);
}

void widget_circle(widget_ptr w, int x, int y, int r, int c)
{
    circleColor(screen, w->globalx + x, w->globaly + y, r, colourgfx[c]);
}

void widget_blit(widget_ptr rect, int x, int y, image_ptr img)
{
    SDL_Rect r;

    r.x = rect->globalx + x;
    r.y = rect->globaly + y;
    SDL_BlitSurface(img, NULL, screen, &r);
}

void image_blit_from_screen(image_ptr img, rect_ptr r)
{
    SDL_BlitSurface(screen, r, img, NULL);
}

void widget_translate(widget_ptr wid, int x, int y)
{
    wid->localx += x;
    wid->localy += y;
    wid->globalx += x;
    wid->globaly += y;
}

void widget_clip(widget_ptr wid)
{
    rect_t r;
    r->x = wid->globalx;
    r->y = wid->globaly;
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
	child->globalx = w->globalx + child->localx;
	child->globaly = w->globaly + child->localy;
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
	if (local_contains(w1, x, y)) {
	    w1->handle_mousebuttondown(w1, button, x - w1->localx, y - w1->localy);
	    return;
	}
    }
}

void widget_show(widget_ptr w)
{
    darray_append(w->parent->show_list, w);
    w->globalx = w->localx + w->parent->globalx;
    w->globaly = w->localy + w->parent->globaly;
}

void widget_hide(widget_ptr w)
{
    darray_remove(w->parent->show_list, w);
}

void widget_draw_children(widget_ptr w)
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
    w->handle_mousebuttondown = widget_handle_mousebuttondown;
    w->handle_keydown = widget_handle_keydown;
    w->update = widget_draw_children;
    w->put_size = widget_put_size;
}

void widget_clear(widget_ptr w)
{
    darray_clear(w->child);
    darray_clear(w->show_list);
}

void image_free(image_ptr img)
{
    SDL_FreeSurface(img);
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

void widget_put_location(widget_ptr wid, int x, int y)
{
    wid->localx = x;
    wid->localy = y;
}

void widget_put_geometry(widget_ptr wid, int x, int y, int w, int h)
{
    wid->localx = x;
    wid->localy = y;
    wid->w = w;
    wid->h = h;
}

SDL_Rect request[100];
int requestcount = 0;

void append_request(int x, int y, int w, int h)
{
    request[requestcount].x = x;
    request[requestcount].y = y;
    if (x + w >= screen->w) request[requestcount].w = screen->w - x;
    else request[requestcount].w = w;
    if (y + h >= screen->h) request[requestcount].h = screen->h - y;
    else request[requestcount].h = h;
    requestcount++;
    if (requestcount >= 100) {
	printf("request overflow!\n");
	requestcount = 0;
    }
}

void request_process()
{
    SDL_UpdateRects(screen, requestcount, request);
    requestcount = 0;
}

static darray_t motioncblist; //TODO: use hash table + linked list instead?
static darray_t keydowncblist;

void widget_system_init()
{
    darray_init(motioncblist);
    darray_init(keydowncblist);
}

void widget_bind_mouse_motion(widget_ptr w, motioncb func, void *data)
{
    widget_callback_ptr p
	= (widget_callback_ptr) malloc(sizeof(widget_callback_t));
    p->w = w;
    p->func = (void *) func;
    p->data = data;
    darray_append(motioncblist, p);
}

void widget_unbind_mouse_motion(widget_ptr w)
{
    int test(void *data) {
	widget_callback_ptr p = (widget_callback_ptr) data;
	return p->w == w;
    }
    darray_remove_with_test(motioncblist, test);
}

void root_mouse_motion(int x0, int y0, int x1, int y1)
{
    void check_item(void *data) {
	widget_callback_ptr p = data;
	if (global_contains(p->w, x0, y0) || global_contains(p->w, x1, y1)) {
	    ((motioncb) p->func)(p->w,
		    x0 - p->w->globalx, y0 - p->w->globaly,
		    x1 - p->w->globalx, y1 - p->w->globaly,
		    p->data);
	}
    }
    darray_forall(motioncblist, check_item);
}

static buttoncbfunc buttonupcb = NULL;
static void *buttonupcbdata = NULL;
static widget_ptr buttonupcbw = NULL;

void widget_on_next_button_up(widget_ptr w, buttoncbfunc func, void *data)
{
    buttonupcbw = w;
    buttonupcb = func;
    buttonupcbdata = data;
}

void root_button_up(widget_ptr w, int button, int x, int y)
{
    if (buttonupcb) {
	buttonupcb(buttonupcbw, button,
		x - buttonupcbw->globalx,
		y - buttonupcbw->globaly,
		buttonupcbdata);
	buttonupcb = NULL;
    }
}

void root_key_down(int sym, int mod)
{
    int i;
    for (i=keydowncblist->count-1; i>=0; i--) {
	widget_callback_ptr p = keydowncblist->item[i];
	if (!((keydowncb) p->func)(p->w, sym, mod, p->data)) return;
    }
}

void widget_push_keydowncb(widget_ptr w, keydowncb func, void *data)
{
    widget_callback_ptr p
	= (widget_callback_ptr) malloc(sizeof(widget_callback_t));
    p->w = w;
    p->func = (void *) func;
    p->data = data;
    darray_append(keydowncblist, p);
}

void widget_pop_keydowncb()
{
    if (darray_is_empty(keydowncblist)) {
	printf("error: keydowncblist is empty!\n");
	return;
    }
    darray_remove_last(keydowncblist);
}
