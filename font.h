#ifndef FONT_H
#define FONT_H

#include <SDL.h>

void font_init();
SDL_Surface *font_rendertext(char *s);
void font_size_text(char *s, int *x, int *y);

#endif //FONT_H
