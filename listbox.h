#ifndef LISTBOX_H
#define LISTBOX_H

#include "darray.h"
#include "widget.h"

struct listbox_s {
    widget_t widget;
    SDL_Surface *image;
};

typedef struct listbox_s *listbox_ptr;
typedef struct listbox_s listbox_t[1];

void listbox_init(listbox_ptr);
void listbox_clear(listbox_ptr);
listbox_ptr listbox_new();
void listbox_add_text(listbox_ptr b, char *s);
void listbox_next(listbox_ptr b);
void listbox_prev(listbox_ptr b);

#endif //LISTBOX_H
