#include <stdlib.h>
#include "button.h"

void button_update(button_ptr b)
{
    widget_ptr w = b->w;
    if (state == state_button_pushed && button_selection == b) {
	widget_lowered_border(w);
    } else {
	widget_raised_border(w);
    }
    widget_blit(w, 2, 2, b->img);
    /*
    if (widget_contains(w, lastmousex, lastmousey)) {
	widget_rectangle(w, -1, -1,  w->w, w->h, c_select);
	widget_string(compan, 10, compan->h - 12, b->text, c_text);
    }
    */
}

void button_handle_mousebuttondown(widget_ptr w, int button, int x, int y)
{
    button_ptr b = (button_ptr) w;
    state = state_button_pushed;
    button_selection = b;
    w->update(w);
}

void button_init(button_ptr b, widget_ptr parent)
{
    widget_init(b->w, parent);
    b->w->update = (void (*)(widget_ptr)) button_update;
    b->w->handle_mousebuttondown = button_handle_mousebuttondown;
}

button_ptr button_new(widget_ptr parent)
{
    button_ptr res = (button_ptr) malloc(sizeof(button_t));
    button_init(res, parent);
    return res;
}

void button_make_text_image(button_ptr b, char *s)
{
    int x, y;
    x = strlen(s) * 8;
    y = 8;

    b->w->w = x + 4;
    b->w->h = y + 4;
    b->img = image_new(x, y);
    image_box_rect(b->img, c_background);
    image_string(b->img, 0, 0, s, c_text);
}
