#include "widget.h"
#include "SDL_gfxPrimitives.h"

static int default_widget_handle_event(widget_ptr w, event_ptr e)
{
    return 0;
}

int widget_handle_event(widget_ptr w, event_ptr e)
{
    if (w->visible) return w->handle_event(w, e);
    return 0;
}

void widget_notify_move(widget_ptr w)
{
    w->moved(w);
}

void widget_notify_resize(widget_ptr w)
{
    widget_raise_signal(w, signal_resize);
}

void widget_moved(widget_ptr w)
{
    if (w->parent) {
	w->x = w->localx + w->parent->x;
	w->y = w->localy + w->parent->y;
    } else {
	w->x = w->localx;
	w->y = w->localy;
    }
}

static void default_widget_update(widget_ptr w)
{
}

void widget_update(widget_ptr w)
{
    if (w->visible) w->update(w);
}

void widget_clear(widget_ptr w)
{
}

void widget_show(widget_ptr w)
{
    w->visible = 1;
}

void widget_hide(widget_ptr w)
{
    w->visible = 0;
}

void widget_init(widget_ptr w)
{
    int i;

    w->has_focus = 0;
    w->can_focus = 0;
    w->handle_event = default_widget_handle_event;
    w->update = default_widget_update;
    w->moved = widget_moved;
    w->parent = NULL;
    w->localx = 0; w->localy = 0;
    w->x = 0; w->y = 0; w->w = 0; w->h = 0;
    for (i=0; i<signal_count; i++) {
	w->handler[i].function = NULL;
	w->handler[i].data = NULL;
    }
    widget_show(w);
}

void widget_put_local(widget_ptr wid, int x, int y)
{
    wid->localx = x;
    wid->localy = y;
    wid->moved(wid);
}

void widget_put_size(widget_ptr wid, int w, int h)
{
    wid->w = w;
    wid->h = h;
    widget_notify_resize(wid);
}

void widget_fill(widget_ptr wid, int c)
{
    SDL_Rect r2;
    r2.x = wid->x;
    r2.y = wid->y;
    r2.w = wid->w;
    r2.h = wid->h;
    SDL_FillRect(screen, &r2, colour[c]);
}

void widget_fillrect(widget_ptr wid, SDL_Rect *r, int c)
{
    if (r) {
	SDL_Rect r2;
	r2.x = r->x + wid->x;
	r2.y = r->y + wid->y;
	r2.w = r->w;
	r2.h = r->h;
	SDL_FillRect(screen, &r2, colour[c]);
    } else {
	widget_fill(wid, c);
    }
}

void widget_raise_signal(widget_ptr w, int sig)
{
    void (*f)(widget_ptr, void *);

    f = w->handler[sig].function;
    if (f) {
	f(w, w->handler[sig].data);
    }
}

void widget_lose_focus(widget_ptr w)
{
    w->has_focus = 0;
    widget_raise_signal(w, signal_lose_focus);
}

int in_widget(widget_ptr wid, int x, int y)
{
    return (wid->x <= x && x < wid->x + wid->w
		&& wid->y <= y && y < wid->y + wid->h);
}

void widget_blit(void *p, SDL_Surface *s, SDL_Rect *src, SDL_Rect *dst)
{
    widget_ptr wid = (widget_ptr) p;
    SDL_Rect r;

    if (dst) {
	r.x = wid->x + dst->x;
	r.y = wid->y + dst->y;
    } else {
	r.x = wid->x;
	r.y = wid->y;
    }
    SDL_BlitSurface(s, src, screen, &r);
}

void widget_connect(widget_ptr w, int sig, callback_f f, void *data)
{
    w->handler[sig].function = f;
    w->handler[sig].data = data;
}

static int lastmousex, lastmousey;
static int lastmod;

void update_mousestate()
{
    SDL_GetMouseState(&lastmousex, &lastmousey);
}

void update_modstate()
{
    lastmod = SDL_GetModState();
}

void widget_getmousexy(widget_ptr w, int *x, int *y)
{
    *x = lastmousex - w->x;
    *y = lastmousey - w->y;
}

int widget_has_mouse(widget_ptr wid)
{
    if (!wid->visible) return 0;
    return in_widget(wid, lastmousex, lastmousey);
}

int widget_getmod(widget_ptr w)
{
    return lastmod;
}

int colourgfx(int c)
    // SDL_gfx has its own colour format
{
    Uint32 i = (rgb[c].r << 24) + (rgb[c].g << 16) + (rgb[c].b << 8) + 255;
    return i;
}

void widget_line(widget_ptr w, int x1, int y1, int x2, int y2, int c)
{
    Uint32 i = colourgfx(c);
    aalineColor(screen, w->x + x1, w->y + y1, w->x + x2, w->y + y2, i);
}

void widget_circle(widget_ptr w, int x, int y, int r, int c)
{
    Uint32 gc = colourgfx(c);
    circleColor(screen, w->x + x, w->y + y, r, gc);
}

void widget_filled_circle(widget_ptr w, int x, int y, int r, int c)
{
    Uint32 gc = colourgfx(c);
    filledCircleColor(screen, w->x + x, w->y + y, r, gc);
}

void widget_filled_polygon(widget_ptr w, int *x, int *y, int n, int c)
{
    Uint32 gc = colourgfx(c);
    Sint16 vx[n], vy[n];
    int i;

    for (i=0; i<n; i++) {
	vx[i] = w->x + x[i];
	vy[i] = w->y + y[i];
    }
    filledPolygonColor(screen, vx, vy, n, gc);
}

void widget_rectangle(widget_ptr w, int x1, int y1, int x2, int y2, int c)
{
    rectangleColor(screen, w->x + x1, w->y + y1,
	    w->x + x2, w->y + y2, colourgfx(c));
}

void widget_write(widget_ptr w, int x, int y, char *s)
{
    SDL_Surface *img;
    SDL_Rect dst;

    dst.x = x;
    dst.y = y;

    img = font_rendertext(s);
    widget_blit(w, img, NULL, &dst);
    SDL_FreeSurface(img);
}

void alt_widget_write(widget_ptr w, int x, int y, char *s)
{
    Uint32 gc = colourgfx(c_text);
    stringColor(screen, w->x + x, w->y + y, s, gc);
}

SDL_Surface *new_image(int w, int h)
{
    SDL_Surface *img;
    SDL_PixelFormat *fmt = screen->format;
    img = SDL_CreateRGBSurface(0, w, h,
	    fmt->BitsPerPixel,
	    fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
    return img;
}
