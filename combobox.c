#include <stdlib.h>
#include "combobox.h"

int listmenu_handle_event(widget_ptr w, event_ptr e)
{
    int x, y;
    int i;
    listmenu_ptr lm = (listmenu_ptr) w;

    switch (e->type) {
	case SDL_MOUSEBUTTONDOWN:
	    if (widget_has_mouse((widget_ptr) lm)) {
		widget_focus(NULL);
		widget_getmousexy((widget_ptr) lm, &x, &y);
		i = y / 16;
		combobox_put_text(lm->combobox, (char *) lm->combobox->choice->item[i]);
		widget_raise_signal((widget_ptr) lm->combobox, signal_activate);
		return 1;
	    }
	    break;
    }
    return 0;
}

void listmenu_update(widget_ptr w)
{
    listmenu_ptr lm = (listmenu_ptr) w;
    widget_blit(w, lm->image, NULL, NULL);
}

void listmenu_popup(listmenu_ptr lm)
{
    int i, n;
    int y;
    int wmax = 0;
    darray_ptr a = lm->combobox->choice;
    SDL_Surface *imgchoice;
    SDL_Rect dst;

    if (lm->image) SDL_FreeSurface(lm->image);

    y = 0;
    n = a->count;
    for (i=0; i<n; i++) {
	int w, h;
	char *s = a->item[i];
	font_size_text(s, &w, &h);
	if (wmax < w) wmax = w;
	y+=16;
    }

    lm->image = new_image(wmax + 10, y + 2);

    dst.x = 0;
    dst.y = 0;
    for (i=0; i<n; i++) {
	char *s = a->item[i];
	imgchoice = font_rendertext(s);
	SDL_BlitSurface(imgchoice, NULL, lm->image, &dst);
	SDL_FreeSurface(imgchoice);
	dst.y+=16;
    }

    widget_put_size((widget_ptr) lm, lm->image->w, lm->image->h);
    widget_put_local((widget_ptr) lm, ((widget_ptr) lm->combobox)->x, ((widget_ptr) lm->combobox)->y);
    widget_focus((widget_ptr) lm);
}

void combobox_update(widget_ptr w)
{
    combobox_ptr b = (combobox_ptr) w;
    SDL_Rect rect;
    widget_draw_inverse_border(w);
    rect.x = 2;
    rect.y = 2;
    rect.w = w->w - 4;
    rect.h = w->h - 4;
    widget_fillrect(w, &rect, c_textbg);
    rect.x = 3;
    rect.y = 3;
    if (b->image) {
	widget_blit(w, b->image, NULL, &rect);
    }
}

int combobox_handle_event(widget_ptr w, event_ptr e)
{
    combobox_ptr b = (combobox_ptr) w;
    switch (e->type) {
	case SDL_MOUSEBUTTONDOWN:
	    if (widget_has_mouse(w)) {
		if (!darray_is_empty(b->choice)) {
		    listmenu_popup(b->listmenu);
		}
		return 1;
	    }
	    break;
    }
    return 0;
}

void listmenu_init(listmenu_ptr lm, combobox_ptr b)
{
    widget_ptr w = (widget_ptr) lm;

    widget_init(w);
    w->can_focus = 1;
    lm->combobox = b;
    lm->image = NULL;
    w->update = listmenu_update;
    w->handle_event = listmenu_handle_event;
    w->can_focus = 1;
}

void combobox_init(combobox_ptr b)
{
    widget_ptr w = (widget_ptr) b;
    widget_init(w);
    w->update = combobox_update;
    w->handle_event = combobox_handle_event;
    b->image = NULL;
    b->text = NULL;

    listmenu_init(b->listmenu, b);
}

combobox_ptr combobox_new()
{
    combobox_ptr b;
    b = (combobox_ptr) malloc(sizeof(struct combobox_s));
    combobox_init(b);
    return b;
}

void combobox_put_text(combobox_ptr b, char *s)
{
    if (b->image) SDL_FreeSurface(b->image);
    if (s) b->image = font_rendertext(s);
    b->text = s;
}

void combobox_clear(combobox_ptr b)
{
    widget_ptr w = (widget_ptr) b;
    if (b->image) SDL_FreeSurface(b->image);
    widget_clear(w);
}
