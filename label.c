#include <stdlib.h>
#include "label.h"

void label_update(widget_ptr w)
{
    label_ptr l = (label_ptr) w;
    widget_string(w, 0, 0, l->text, c_text);
}

void label_init(label_ptr l, widget_ptr parent)
{
    widget_init(l->w, parent);
    l->w->update = label_update;
}

void label_put_text(label_ptr l, char *s)
{
    l->text = s; //TODO: strclone it?
    l->w->w = strlen(s) * 8;
    l->w->h = 8;
}

label_ptr label_new(widget_ptr parent)
{
    label_ptr res = (label_ptr) malloc(sizeof(label_t));
    label_init(res, parent);
    return res;
}
