#ifndef CONTAINER_H
#define CONTAINER_H

#include "darray.h"
#include "widget.h"

struct container_s {
    widget_t widget;
    darray_t child;
};

typedef struct container_s *container_ptr;
typedef struct container_s container_t[1];

void container_init(container_ptr);
void container_clear(container_ptr);
void container_put_widget(container_ptr con, widget_ptr w, int x, int y);
container_ptr container_new();

#endif //CONTAINER_H
