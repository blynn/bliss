#include <SDL.h>
#include "colour.h"

int colour[c_max];
SDL_Color rgb[c_max];

static void put_colour(int i, int r, int g, int b)
{
    rgb[i].r = r;
    rgb[i].g = g;
    rgb[i].b = b;
}

void init_colour(SDL_PixelFormat *format)
{
    int i;

    put_colour(c_background, 96, 96, 96);
    put_colour(c_textbg, 0, 0, 0);
    put_colour(c_menuborder, 127, 127, 191);
    put_colour(c_menubg, 0, 0, 127);
    put_colour(c_menubghi, 63, 127, 127);
    put_colour(c_generator, 0, 0, 127);
    put_colour(c_master, 0, 127, 0);
    put_colour(c_effect, 127, 0, 0);
    put_colour(c_border, 255, 255, 255);
    put_colour(c_text, 255, 255, 255);
    put_colour(c_liveedge, 255, 0, 0);
    put_colour(c_edge, 255, 255, 255);
    put_colour(c_arrow, 255, 191, 191);
    put_colour(c_edgedisc, 191, 0, 0);
    put_colour(c_cursor, 127, 127, 255);
    put_colour(c_titlebar, 0, 127, 127);
    put_colour(c_gridline, 127, 127, 127);
    put_colour(c_machine_cursor, 64, 255, 64);
    put_colour(c_mabg, 0, 0, 0);
    put_colour(c_dark, 63, 63, 63);
    put_colour(c_darker, 0, 0, 0);
    put_colour(c_light, 160, 160, 160);
    put_colour(c_lighter, 255, 255, 255);

    for (i=0; i<c_max; i++) {
	colour[i] = SDL_MapRGB(format, rgb[i].r, rgb[i].g, rgb[i].b);
    }
}
