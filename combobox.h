#ifndef LISTBOX_H
#define LISTBOX_H

#include "darray.h"
#include "widget.h"

struct combobox_s;

struct listmenu_s {
    widget_t widget;
    struct combobox_s *combobox;
    SDL_Surface *image;
};
typedef struct listmenu_s *listmenu_ptr;
typedef struct listmenu_s listmenu_t[1];

struct combobox_s {
    widget_t widget;
    char *text;
    SDL_Surface *image;
    darray_ptr choice;
    listmenu_t listmenu;
};

typedef struct combobox_s *combobox_ptr;
typedef struct combobox_s combobox_t[1];

void combobox_init(combobox_ptr);
void combobox_clear(combobox_ptr);
combobox_ptr combobox_new();
void combobox_put_text(combobox_ptr b, char *s);
void combobox_next(combobox_ptr b);
void combobox_prev(combobox_ptr b);

#endif //LISTBOX_H
