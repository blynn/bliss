#ifndef LABEL_H
#define LABEL_H

#include "widget.h"

struct label_s {
    widget_t w;
    char *text;
};
typedef struct label_s label_t[1];
typedef struct label_s *label_ptr;

extern label_ptr label_selection;

void label_update(widget_ptr w);
void label_init(label_ptr l, widget_ptr parent);
void label_clear(label_ptr l);
void label_put_text(label_ptr l, char *s);
label_ptr label_new(widget_ptr parent);

#endif //LABEL_H
