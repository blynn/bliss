#include <stdlib.h>
#include <string.h>
#include "textbox.h"
#include "util.h"

enum {
    init_textmax = 128,
};

void textbox_update(widget_ptr w)
{
    textbox_ptr b = (textbox_ptr) w;
    SDL_Rect rect;
    widget_draw_inverse_border(w);
    rect.x = 2;
    rect.y = 2;
    rect.w = w->w - 4;
    rect.h = w->h - 4;
    widget_fillrect(w, &rect, c_textbg);
    if (b->image) {
	widget_blit(w, b->image, NULL, &rect);
    }
    rect.x = 3;
    rect.y = 3;
    if (w->has_focus || b->appear_active) {
	rect.x += b->cursorx;
	rect.y = 0;
	rect.w = 1;
	rect.h = w->h;
	widget_fillrect(w, &rect, c_text);
    }
}

static void updatecursor(textbox_ptr b)
{
    if (b->image) {
	char *s = (char *) alloca(b->cursor + 2);
	int y;
	strncpy(s, b->text, b->cursor);
	s[b->cursor] = 0;
	font_size_text(s, &b->cursorx, &y);
    } else {
	b->cursorx = 0;
    }
}

static void updateimg(textbox_ptr b)
{
    if (b->image) {
	SDL_FreeSurface(b->image);
	b->image = NULL;
    }
    if (b->textlen) {
	b->image = font_rendertext(b->text);
    }
    updatecursor(b);
}

void textbox_left(textbox_ptr b)
{
    if (b->cursor > 0) b->cursor--;
    updatecursor(b);
}

void textbox_right(textbox_ptr b)
{
    if (b->cursor < b->textlen) b->cursor++;
    updatecursor(b);
}

void textbox_insert_ch(textbox_ptr b, char c)
{
    if (b->textlen + 1 >= b->textmax) {
	b->textmax += init_textmax;
	b->text = (char *) realloc(b->text, b->textmax);
    }
    memmove(&b->text[b->cursor + 1], &b->text[b->cursor],
	    b->textlen - b->cursor + 1); // + 1 for the NULL at the end
    b->text[b->cursor] = c;
    b->cursor++;
    b->textlen++;
    updateimg(b);
}

void textbox_delete(textbox_ptr b)
{
    if (b->cursor >= b->textlen) return;
    memmove(&b->text[b->cursor], &b->text[b->cursor + 1],
	    b->textlen - b->cursor);
    b->textlen--;
    updateimg(b);
}

void textbox_backspace(textbox_ptr b)
{
    if (!b->cursor) return;
    b->cursor--;
    textbox_delete(b);
}

int textbox_handle_key(widget_ptr w, int key)
{
    textbox_ptr b = (textbox_ptr) w;
    switch(key) {
	case SDLK_DELETE:
	    textbox_delete(b);
	    break;
	case SDLK_BACKSPACE:
	    textbox_backspace(b);
	    break;
	case SDLK_LEFT:
	    textbox_left(b);
	    break;
	case SDLK_RIGHT:
	    textbox_right(b);
	    break;
	case SDLK_ESCAPE:
	    widget_raise_signal(w, signal_cancel);
	    break;
	case SDLK_RETURN:
	    widget_raise_signal(w, signal_activate);
	    break;
	default:
	    if (key >= 32 && key <= 255) {
		if (widget_getmod(w) & KMOD_SHIFT) {
		    textbox_insert_ch(b, shift_key(key));
		} else textbox_insert_ch(b, key);
	    }
	    break;
    }
    return 1;
}

int textbox_handle_event(widget_ptr w, event_ptr e)
{
    if (e->type == SDL_KEYDOWN) {
	return textbox_handle_key(w, e->key.keysym.sym);
    } else return 0;
}

void textbox_init(textbox_ptr b)
{
    widget_ptr w = (widget_ptr) b;
    widget_init(w);
    w->update = textbox_update;
    w->handle_event = textbox_handle_event;
    w->can_focus = 1;
    b->image = NULL;
    b->textmax = init_textmax;
    b->text = (char *) malloc(b->textmax);
    b->text[0] = 0;
    b->textlen = 0;
    b->cursor = 0;
    b->appear_active = 0;
    updateimg(b);
}

textbox_ptr textbox_new()
{
    textbox_ptr b;
    b = (textbox_ptr) malloc(sizeof(struct textbox_s));
    textbox_init(b);
    return b;
}

void textbox_put_text(textbox_ptr b, char *s)
{
    if (s) {
	b->textlen = strlen(s);
	b->cursor = b->textlen;
	if (b->textlen > b->textmax) {
	    b->text = (char *) realloc(b->text, b->cursor + init_textmax);
	}
	strcpy(b->text, s);
	updateimg(b);
    } else {
	//save some memory if possible
	if (b->textmax > init_textmax) b->text = (char *) realloc(b->text, init_textmax);
	b->text[0] = 0;
	b->textlen = 0;
	b->cursor = 0;
	updateimg(b);
    }
}

void textbox_clear(textbox_ptr b)
{
    widget_ptr w = (widget_ptr) b;
    if (b->image) SDL_FreeSurface(b->image);
    free(b->text);
    widget_clear(w);
}
