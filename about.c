#include "version.h"
#include "window.h"
#include "label.h"
#include "button.h"

void okcb(void *data)
{
    window_ptr win = (window_ptr) data;
    window_close(win);
}

void about_init(window_ptr win, widget_ptr parent)
{
    widget_ptr w = win->w;
    button_ptr b;
    label_ptr l;

    window_init(win, parent);
    w->localx = 50;
    w->localy = 50;
    w->put_size(w, 120, 90);
    win->title = "About";

    b = button_new(win->body);
    w = b->w;
    w->localx = 45;
    w->localy = 55;
    w->w = 16 + 4;
    w->h = 8 + 4;
    b->text = "Ok";
    b->img = image_new(16, 8);
    image_box_rect(b->img, c_background);
    image_string(b->img, 0, 0, b->text, c_text);
    b->callback = okcb;
    b->data = (void *) win;
    widget_show(b->w);

    l = label_new(win->body);
    l->text = "Bliss " VERSION_STRING;
    l->w->localx = 5;
    l->w->localy = 10;
    widget_show(l->w);

    l = label_new(win->body);
    l->text = "by Ben Lynn";
    l->w->localx = 5;
    l->w->localy = 30;
    widget_show(l->w);
}
