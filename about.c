#include <string.h>
#include "label.h"
#include "button.h"
#include "window.h"
#include "version.h"

static void hide_cb(widget_ptr caller, void *data)
{
    window_close((window_ptr) data);
}

window_ptr about_new()
{
    char buf[128];
    window_ptr win = window_new();
    widget_ptr w = (widget_ptr) win;
    label_ptr l;
    button_ptr b = button_new();
    SDL_Surface *img;

    widget_put_size(w, 150, 100);
    img = font_rendertext("Ok");
    button_put_image(b, img);
    button_shrinkwrap(b);
    window_put_widget(win, (widget_ptr) b, 70, 60);
    widget_connect((widget_ptr) b, signal_activate, hide_cb, win);

    l = label_new();
    strcpy(buf, "Bliss v");
    strcat(buf, bliss_version_string());
    label_put_text(l, buf);
    window_put_widget(win, (widget_ptr) l, 40, 10);

    l = label_new();
    label_put_text(l, "by Ben Lynn");
    window_put_widget(win, (widget_ptr) l, 35, 30);

    window_put_title(win, "About");

    return win;
}
