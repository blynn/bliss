#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "widget.h"

struct textbox_s {
    widget_t widget;
    int textlen;
    int textmax;
    char *text;
    int cursor;
    int cursorx;
    SDL_Surface *image;
    int appear_active;
};

typedef struct textbox_s *textbox_ptr;
typedef struct textbox_s textbox_t[1];

void textbox_init(textbox_ptr);
void textbox_clear(textbox_ptr);
textbox_ptr textbox_new();
void textbox_put_text(textbox_ptr b, char *s);
int textbox_handle_key(widget_ptr w, int key);

#endif //TEXTBOX_H
