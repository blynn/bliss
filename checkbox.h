#ifndef CHECKBOX_H
#define CHECKBOX_H

#include "widget.h"

struct checkbox_s {
    widget_t w;
    void (*callback)(void *data, int state);
    char *data;
    int state;
};
typedef struct checkbox_s checkbox_t[1];
typedef struct checkbox_s *checkbox_ptr;

void checkbox_update(checkbox_ptr b);
void checkbox_init(checkbox_ptr b, widget_ptr parent);
checkbox_ptr checkbox_new(widget_ptr parent);

#endif //CHECKBOX_H
