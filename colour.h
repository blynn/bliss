#ifndef COLOUR_H
#define COLOUR_H

#include <SDL.h>

enum {
    c_background = 0,
    c_generator,
    c_effect,
    c_master,
    c_border,
    c_menuborder,
    c_menubg,
    c_menubghi,
    c_text,
    c_liveedge,
    c_edge,
    c_arrow,
    c_edgedisc,
    c_cursor,
    c_titlebar,
    c_gridline,
    c_max
};

extern int colour[c_max];
extern SDL_Color rgb[c_max];

void init_colour(SDL_PixelFormat *format);

#endif //COLOUR_H
