#include <SDL.h>
#include "colour.h"

int coloursdl[c_max];
SDL_Color rgb[c_max];
int colourgfx[c_max];

static void put_colour(int i, int r, int g, int b)
{
    rgb[i].r = r;
    rgb[i].g = g;
    rgb[i].b = b;
}

void colour_init(SDL_PixelFormat *format)
{
    int i;

    put_colour(c_background, 207, 207, 207);
    put_colour(c_highlight, 255, 255, 255);
    put_colour(c_shadow, 143, 143, 143);
    put_colour(c_darkshadow, 0, 0, 0);
    put_colour(c_text, 0, 0, 0);
    put_colour(c_invtext, 255, 255, 255);
    put_colour(c_emphasis, 0, 255, 255);
    put_colour(c_select, 0, 255, 0);
    put_colour(c_canvas, 64, 64, 64);
    put_colour(c_menubg, 80, 100, 150);
    put_colour(c_textboxbg, 255, 255, 255);
    put_colour(c_unit, 0, 127, 127);
    put_colour(c_darkunit, 64, 96, 64);
    put_colour(c_porttext, 176, 176, 176);
    put_colour(c_edge, 255, 255, 255);
    put_colour(c_darkedge, 176, 176, 176);
    put_colour(c_led, 255, 64, 64);

    for (i=0; i<c_max; i++) {
	coloursdl[i] = SDL_MapRGB(format, rgb[i].r, rgb[i].g, rgb[i].b);
	colourgfx[i] = (rgb[i].r << 24) + (rgb[i].g << 16) + (rgb[i].b << 8) + 255;
    }
}
