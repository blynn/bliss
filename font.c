#include <stdlib.h>
#include <SDL_ttf.h>
#include "colour.h"
#include "font.h"

static TTF_Font *font;

void font_init()
{
    int status;

    status = TTF_Init();
    if (status) {
	fprintf(stderr, "init: TTF_Init failed: %s\n", SDL_GetError());
	exit(-1);
    }
    atexit(TTF_Quit);

    //font = TTF_OpenFont("/usr/share/fonts/truetype/Arial.ttf", 12);
    font = TTF_OpenFont("helmetr.ttf", 12);
    if (!font) {
	fprintf(stderr, "init: TTF_OpenFont failed: %s\n", SDL_GetError());
	exit(-1);
    }
    TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
}

SDL_Surface *font_rendertext(char *s)
{
    SDL_Surface *r, *tmp;

    tmp = TTF_RenderText_Solid(font, s, rgb[c_text]);
    r = SDL_DisplayFormat(tmp);
    SDL_FreeSurface(tmp);

    return r;
}

void font_size_text(char *s, int *x, int *y)
{
    TTF_SizeText(font, s, x, y);
}
