#ifndef BUTTON_H
#define BUTTON_H

#include "widget.h"

struct button_s {
    widget_t w;
    char *text;
    char hotkey;
    void (*callback)(void *data);
    char *data;
    image_ptr img;
};
typedef struct button_s button_t[1];
typedef struct button_s *button_ptr;

extern button_ptr button_selection;

void button_update(button_ptr b);
void button_init(button_ptr b, widget_ptr parent);
button_ptr button_new(widget_ptr parent);
void button_make_text_image(button_ptr b, char *s);

#endif //BUTTON_H
