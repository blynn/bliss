#include "widget.h"

struct menu_s;
struct menu_entry_s {
    widget_t w;
    int pushed;
    struct menu_s *m;
    char *text;
    void (*callback)(void *);
    void *data;
};
typedef struct menu_entry_s menu_entry_t[1];
typedef struct menu_entry_s *menu_entry_ptr;

struct menu_s {
    widget_t w;
    widget_ptr selected;
    int wmax;
    image_ptr under;
};
typedef struct menu_s menu_t[1];
typedef struct menu_s *menu_ptr;

void menu_init(menu_ptr m, widget_ptr parent);

struct menubutton_s {
    widget_t w;
    char *text;
    menu_t menu;
    int pushed;
};
typedef struct menubutton_s menubutton_t[1];
typedef struct menubutton_s *menubutton_ptr;

void menubutton_init(menubutton_ptr m, widget_ptr parent, char *s);

struct menubar_s {
    widget_t w;
    widget_ptr selected;
};
typedef struct menubar_s menubar_t[1];
typedef struct menubar_s *menubar_ptr;

void menubar_init(menubar_ptr m, widget_ptr parent);
menu_ptr menubar_add_button(menubar_ptr m, char *s);
void menu_add_command(menu_ptr m, char *s, void (*func)(void *), void *data);
