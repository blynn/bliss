#ifndef LISTBOX_H
#define LISTBOX_H

#include "darray.h"
#include "widget.h"

struct listbox_s;

struct listmenu_s {
    widget_t widget;
    struct listbox_s *listbox;
    SDL_Surface *image;
};
typedef struct listmenu_s *listmenu_ptr;
typedef struct listmenu_s listmenu_t[1];

struct listbox_s {
    widget_t widget;
    char *text;
    SDL_Surface *image;
    darray_ptr choice;
    listmenu_t listmenu;
};

typedef struct listbox_s *listbox_ptr;
typedef struct listbox_s listbox_t[1];

void listbox_init(listbox_ptr);
void listbox_clear(listbox_ptr);
listbox_ptr listbox_new();
void listbox_put_text(listbox_ptr b, char *s);
void listbox_next(listbox_ptr b);
void listbox_prev(listbox_ptr b);

#endif //LISTBOX_H
