#include <stdlib.h>
#include "button.h"

void button_update(button_ptr b)
{
    widget_ptr w = b->w;
    if (b->pushed) {
	widget_lowered_border(w);
    } else {
	widget_raised_border(w);
    }
    widget_blit(w, 2, 2, b->img);
    /*
    if (global_contains(w, lastmousex, lastmousey)) {
	widget_rectangle(w, -1, -1,  w->w, w->h, c_select);
	widget_string(compan, 10, compan->h - 12, b->text, c_text);
    }
    */
}

static void button_button_up(widget_ptr w, int button, int x, int y, void *data)
{
    button_ptr p = (button_ptr) w;
    p->pushed = 0;
    widget_update(w);
    request_update(w);
    if (widget_contains(p->w, x, y)) {
	p->callback(p->data);
    }
}

void button_handle_mousebuttondown(widget_ptr w, int button, int x, int y)
{
    button_ptr b = (button_ptr) w;
    b->pushed = -1;
    widget_update(w);
    request_update(w);
    widget_on_next_button_up(w, button_button_up, NULL);
}

void button_init(button_ptr b, widget_ptr parent)
{
    widget_init(b->w, parent);
    b->w->update = (void (*)(widget_ptr)) button_update;
    b->w->handle_mousebuttondown = button_handle_mousebuttondown;
    b->pushed = 0;
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

void button_put_callback(button_ptr b, void (*func)(void *), void *data)
{
    b->callback = func;
    b->data = data;
}
