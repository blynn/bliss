#ifndef MENU_H
#define MENU_H

#include "darray.h"
#include "widget.h"

enum {
    menubar_h = 16,
};

struct menuitem_s {
    struct widget_s widget;
    char *text;
    SDL_Surface *img;
};
typedef struct menuitem_s menuitem_t[1];
typedef struct menuitem_s *menuitem_ptr;

struct menu_s {
    struct widget_s widget;
    darray_t item;
};
typedef struct menu_s menu_t[1];
typedef struct menu_s *menu_ptr;

struct menubar_s {
    widget_t widget;
    darray_t item;
};

typedef struct menubar_s *menubar_ptr;
typedef struct menubar_s menubar_t[1];

void menuitem_put_text(menuitem_ptr m, char *s);
void menuitem_init(menuitem_ptr m);
void menuitem_set_submenu(menuitem_ptr it, menu_ptr m);

menuitem_ptr menuitem_new();

void menubar_init(menubar_ptr);
void menubar_clear(menubar_ptr m);
void menubar_add(menubar_t m, menuitem_t it);

void menu_init(menu_ptr m);
void menu_clear(menu_ptr m);
void menu_add(menu_ptr m, menuitem_t it);
menu_ptr menu_new();
void menu_popup(menu_ptr);
#endif //MENU_H
