#ifndef COLOUR_H
#define COLOUR_H

#include <SDL.h>

enum {
    c_background = 0,
    c_highlight,
    c_shadow,
    c_darkshadow,
    c_canvas,
    c_menubg,
    c_textboxbg,
    c_emphasis,
    c_select,
    c_text,
    c_invtext,
    c_porttext,
    c_unit,
    c_darkunit,
    c_edge,
    c_darkedge,
    c_max
};

extern int coloursdl[c_max];
extern int colourgfx[c_max];
extern SDL_Color rgb[c_max];

void colour_init(SDL_PixelFormat *format);

#endif //COLOUR_H
