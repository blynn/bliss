#ifndef BUTTON_H
#define BUTTON_H

#include "widget.h"

struct button_s {
    widget_t widget;
    SDL_Surface *image;
};

typedef struct button_s *button_ptr;
typedef struct button_s button_t[1];

void button_init(button_ptr);
void button_clear(button_ptr);
button_ptr button_new();
void button_put_image(button_ptr b, SDL_Surface *img);
void button_shrinkwrap(button_ptr b);

#endif //BUTTON_H
