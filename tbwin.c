//TODO: very kludgy
#include <string.h>
#include <stdlib.h>
#include "label.h"
#include "textbox.h"
#include "button.h"
#include "window.h"
#include "tbwin.h"

static void cancel_cb(widget_ptr caller, void *data)
{
    window_close((window_ptr) data);
}

static void ok_cb(widget_ptr caller, void *data)
{
    window_close((window_ptr) data);
    widget_raise_signal((widget_ptr) data, signal_activate);
}

void tbwin_init(tbwin_ptr t)
{
    window_ptr win = t->win;
    widget_ptr w = (widget_ptr) win;
    button_ptr b;
    SDL_Surface *img;

    window_init(win);

    widget_put_size(w, 210, 70);

    b = button_new();
    img = font_rendertext("Cancel");
    button_put_image(b, img);
    button_shrinkwrap(b);
    window_put_widget(win, (widget_ptr) b, 156, 24);
    widget_connect((widget_ptr) b, signal_activate, cancel_cb, win);

    b = button_new();
    img = font_rendertext("Ok");
    button_put_image(b, img);
    button_shrinkwrap(b);
    window_put_widget(win, (widget_ptr) b, 132, 24);
    widget_connect((widget_ptr) b, signal_activate, ok_cb, win);

    textbox_init(t->tb);
    widget_put_size((widget_ptr) t->tb, 196, 16);
    window_put_widget(win, (widget_ptr) t->tb, 5, 2);

    window_focus(win, (widget_ptr) t->tb);

    widget_connect((widget_ptr) t->tb, signal_activate, ok_cb, win);

    widget_hide(w);
}

void tbwin_open(tbwin_ptr t)
{
    window_modal_open((window_ptr) t);
}

void tbwin_put_title(tbwin_ptr t, char *title)
{
    window_put_title((window_ptr) t, title);
}
