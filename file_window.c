#include "file_window.h"

extern void close_window();

void button_ok(void *data)
{
    textbox_ok((textbox_ptr) data);
}

void button_cancel(void *data)
{
    close_window();
}

void file_window_setup(file_window_ptr fw, char *title, char *buttontext,
	void (*callback)(void *, char *))
{
    button_ptr b;
    textbox_ptr tb;

    fw->win->title = title;
    
    b = fw->bok;
    tb = fw->tbfilename;
    image_free(b->img);
    button_make_text_image(b, buttontext);
    tb->ok_cb = callback;
}

void file_window_init(file_window_ptr fw, widget_ptr parent)
{
    window_ptr win = fw->win;
    widget_ptr w = win->w;
    button_ptr b;
    label_ptr l;
    textbox_ptr tb;

    window_init(win, parent);
    w->localx = 50;
    w->localy = 50;
    w->put_size(w, 300, 90);
    win->title = "File Window";

    tb = fw->tbfilename;
    textbox_init(tb, win->body);
    tb->w->localx = 5;
    tb->w->localy = 30;
    //tb->w->w = win->body->w - 10;
    tb->w->w = 300 - 10 - 4;
    tb->w->h = 16;
    widget_show(tb->w);

    b = fw->bok;
    button_init(b, win->body);
    w = b->w;
    w->localx = 45;
    w->localy = 55;
    b->text = "Ok";
    button_make_text_image(b, "Ok");
    button_put_callback(b, button_ok, (void *) tb);
    widget_show(b->w);

    b = fw->bcancel;
    button_init(b, win->body);
    w = b->w;
    w->localx = 105;
    w->localy = 55;
    b->text = "Cancel";
    button_make_text_image(b, b->text);
    button_put_callback(b, button_cancel, (void *) tb);
    widget_show(b->w);

    l = label_new(win->body);
    l->text = "Filename";
    l->w->localx = 5;
    l->w->localy = 10;
    widget_show(l->w);
}
