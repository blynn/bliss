#include <string.h>

#include "textbox.h"

void textbox_put_string(textbox_t tb, char *s)
{
    //TODO: replace with dynamic version
    tb->s[128] = 0;
    strncpy(tb->s, s, 128);
    tb->cursor = tb->len = strlen(s);
    strcpy(tb->savedcopy, tb->s);
}

void textbox_update(textbox_t tb)
{
    widget_box_rect(tb->w, c_textboxbg);
    
    widget_lowered_border(tb->w);

    widget_string(tb->w, 4, 4, tb->s, c_text);

    if (state == state_textbox && textbox_selection == tb) {
	//draw cursor
	int x;
	x = tb->cursor * 8 + 2;
	widget_box(tb->w, x, 1, x + 1, 12, c_text);
    }
}

void textbox_insert(textbox_t tb, char ch)
{
    memmove(&tb->s[tb->cursor + 1], &tb->s[tb->cursor], tb->len - tb->cursor + 1);
    tb->s[tb->cursor] = ch;
    tb->cursor++;
    tb->len++;
}

void textbox_delete(textbox_t tb)
{
    if (tb->cursor >= tb->len) return;
    memmove(&tb->s[tb->cursor], &tb->s[tb->cursor + 1], tb->len - tb->cursor);
    tb->len--;
}

static char shift_key(unsigned char ch)
{
    static char shifttable[256];
    static int first = 1;

    void add_shiftstring(char *s1, char *s2)
    {
	int i;

	for (i=0; i<strlen(s1); i++) {
	    shifttable[(int) s1[i]] = s2[i];
	}
    }

    if (first) {
	int c;

	for (c=0; c<256; c++) shifttable[c] = c;

	for (c='a'; c<='z'; c++) shifttable[c] = c - 32;

	add_shiftstring("1234567890-=", "!@#$%^&*()_+");
	add_shiftstring("[]\\;',./`", "{}|:\"<>?~");
    }

    return shifttable[ch];
}

void textbox_ok(textbox_t tb)
{
    state = state_normal;
    textbox_selection = NULL;
    strcpy(tb->savedcopy, tb->s);
    textbox_update(tb);
    tb->ok_cb(tb->ok_cb_data);
}

void textbox_cancel(textbox_t tb)
{
    state = state_normal;
    textbox_selection = NULL;
    textbox_put_string(tb, tb->savedcopy);
    textbox_update(tb);
    tb->cancel_cb(tb->cancel_cb_data);
}

void textbox_handlembdown(textbox_t tb, int button, int x, int y)
{
    state = state_textbox;
    textbox_selection = tb;
    tb->cursor = x / 8;
    if (tb->cursor > tb->len) tb->cursor = tb->len;
    textbox_update(tb);
}

void textbox_handlekey(textbox_t tb, int key, int mod)
{
    if (key >= 32 && key <= 126) {
	if (mod & KMOD_SHIFT) textbox_insert(tb, shift_key(key));
	else textbox_insert(tb, key);
    } else switch(key) {
	case SDLK_LEFT:
	    if (tb->cursor) tb->cursor--;
	    break;
	case SDLK_RIGHT:
	    if (tb->cursor < tb->len) tb->cursor++;
	    break;
	case SDLK_HOME:
	    tb->cursor = 0;
	    break;
	case SDLK_END:
	    tb->cursor = tb->len;
	    break;
	case SDLK_BACKSPACE:
	    if (tb->cursor) {
		tb->cursor--;
		textbox_delete(tb);
	    }
	    break;
	case SDLK_DELETE:
	    textbox_delete(tb);
	    break;
	case SDLK_ESCAPE:
	    textbox_cancel(tb);
	    return;
	    break;
	case SDLK_RETURN:
	    textbox_ok(tb);
	    return;
	    break;
    }
    textbox_update(tb);
}

void textbox_init(textbox_ptr tb, widget_ptr parent)
{
    widget_init(tb->w, parent);
    tb->w->update = (void (*)(widget_ptr)) textbox_update;
    tb->w->handle_mousebuttondown = textbox_handlembdown;
    tb->w->handle_keydown = textbox_handlekey;
    textbox_put_string(tb, "");
}

textbox_ptr textbox_new(widget_ptr parent)
{
    textbox_ptr res;

    res = malloc(sizeof(textbox_t));
    textbox_init(res, parent);
    return res;
}
