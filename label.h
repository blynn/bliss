#ifndef LABEL_H
#define LABEL_H

#include "widget.h"

struct label_s {
    widget_t widget;
    SDL_Surface *image;
};

typedef struct label_s *label_ptr;
typedef struct label_s label_t[1];

void label_init(label_ptr);
void label_clear(label_ptr);
label_ptr label_new();
void label_put_text(label_ptr, char *s);
void label_shrinkwrap(label_ptr l);

#endif //LABEL_H
