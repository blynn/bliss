#include <string.h>
#include "label.h"
#include "textbox.h"
#include "button.h"
#include "window.h"
#include "version.h"

static void cancel_cb(widget_ptr caller, void *data)
{
    window_close((window_ptr) data);
}

void perform_file_op(char *filename);

static void filewin_activate_cb(widget_ptr caller, void *data)
{
    textbox_ptr tb = (textbox_ptr) data;
    perform_file_op(tb->text);
}

static void ok_cb(widget_ptr caller, void *data)
{
    window_close((window_ptr) data);
    widget_raise_signal((widget_ptr) data, signal_activate);
}

window_ptr filewin_new()
{
    window_ptr win = window_new();
    widget_ptr w = (widget_ptr) win;
    label_ptr l;
    textbox_ptr tb;
    button_ptr b;
    SDL_Surface *img;

    widget_put_size(w, 210, 90);

    b = button_new();
    img = font_rendertext("Cancel");
    button_put_image(b, img);
    button_shrinkwrap(b);
    window_put_widget(win, (widget_ptr) b, 156, 44);
    widget_connect((widget_ptr) b, signal_activate, cancel_cb, win);

    b = button_new();
    img = font_rendertext("Ok");
    button_put_image(b, img);
    button_shrinkwrap(b);
    window_put_widget(win, (widget_ptr) b, 132, 44);
    widget_connect((widget_ptr) b, signal_activate, ok_cb, win);

    l = label_new();
    label_put_text(l, "File:");
    window_put_widget(win, (widget_ptr) l, 5, 1);

    tb = textbox_new();
    widget_put_size((widget_ptr) tb, 200, 16);
    window_put_widget(win, (widget_ptr) tb, 5, 20);

    widget_connect((widget_ptr) tb, signal_activate, ok_cb, win);

    widget_connect(w, signal_activate, filewin_activate_cb, tb);

    widget_hide(w);

    return win;
}
