#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "SDL_gfxPrimitives.h"
#include "SDL_rotozoom.h"
#include "version.h"

static Uint8 *buf; //circular buffer
static int buf_size;
static int buf_start;
static int buf_empty;
static int buf_end;

static SDL_Surface *screen;
static int glob_bpp;

static TTF_Font *font;
static SDL_Color white = { 0xFF, 0xFF, 0xFF, 0 };

/*
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    static int rmask = 0xff000000;
    static int gmask = 0x00ff0000;
    static int bmask = 0x0000ff00;
    static int amask = 0x000000ff;
#else
    static int rmask = 0x000000ff;
    static int gmask = 0x0000ff00;
    static int bmask = 0x00ff0000;
    static int amask = 0xff000000;
#endif
*/

static int fillcount;

static void c_fill_audio(void *udata, Uint8 *stream, int len) {
	int i;

	if (buf_start == buf_end && buf_empty) {
		printf("buffer underrun\n");
		return;
	}
	for (i=0; i<len;) {
		stream[i] = buf[buf_start];
		buf_start++;
		i++;
		stream[i] = buf[buf_start];
		buf_start++;
		i++;
		fillcount ++;
		if (buf_start >= buf_size) buf_start = 0;
		if (buf_start == buf_end) {
			printf("buffer underrun\n");
			buf_empty = -1;
			return;
		}
	}
}

void append_buf(double x)
{
	int n;
	n = x * 32700;
	if (n > 32767) n = 32767;
	else if (n < -32767) n = -32767;
	buf[buf_end] = (Uint8) (n & 255); //lower 16 bits
	buf_end++;
	n = n >> 8;
	buf[buf_end] = (Uint8) (n & 255); //upper 16 bits
	buf_end++;
	if (buf_end >= buf_size) buf_end = 0;
	if (buf_end == buf_start) buf_empty = 0;
}

int buffer_ready(void)
{
	return (buf_end != buf_start || buf_empty);
}

void buffer_test() {
	static double v = -1.0;
	while (buffer_ready()) {
		append_buf(v);
		append_buf(v);
		v += 0.0002;
		if (v > 1.0) v = -1.0;
	}
}

void audio_test(void) {
	int done = 0;
	SDL_Event event;
	printf("Audio test\n");
	buffer_test();

	SDL_PauseAudio(0);
	do {
		SDL_LockAudio();
		buffer_test();
		SDL_UnlockAudio();
		if (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN) {
				done = 1;
			}
		}
	} while (!done);
	SDL_PauseAudio(1);
}

int fg_colour;

void ext_set_fg_colour(int c)
{
	fg_colour = c;
}

void ext_pixelcolor(int x, int y, int color)
{
    pixelColor(screen, x, y, color);
}

void ext_aalinecolor(int x1, int y1, int x2, int y2, int color)
{
    aalineColor(screen, x1, y1, x2, y2, color);
}

void ext_filledcirclecolor(int x, int y, int r, int color)
{
    filledCircleColor(screen, x, y, r, color);
}

SDL_AudioSpec wanted;

void init(void)
{
	int status;

	status = SDL_Init(SDL_INIT_EVERYTHING);
	if (status) {
		fprintf(stderr, "init: SDL_Init failed: %s\n", SDL_GetError());
		exit(-1);
	}
	atexit(SDL_Quit);

	status = TTF_Init();
	if (status) {
		fprintf(stderr, "init: TTF_Init failed: %s\n", SDL_GetError());
		exit(-1);
	}
	atexit(TTF_Quit);

	font = TTF_OpenFont("/usr/share/fonts/truetype/Arial.ttf", 12);
	//font = TTF_OpenFont("C:\\WINDOWS\\Fonts\\ARIAL.TTF", 12);
	if (!font) {
		fprintf(stderr, "init: TTF_OpenFont failed: %s\n", SDL_GetError());
		exit(-1);
	}
	TTF_SetFontStyle(font, TTF_STYLE_NORMAL);

	/* Set the audio format */
	wanted.freq = 44100;
	wanted.format = AUDIO_S16;
	wanted.channels = 2;    /* 1 = mono, 2 = stereo */
	wanted.samples = 1024;  /* Good low-latency value for callback */
	wanted.callback = c_fill_audio;
	wanted.userdata = NULL;

	/* Open the audio device, forcing the desired format */
	if ( SDL_OpenAudio(&wanted, NULL) < 0 ) {
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
		exit(-1);
	}

	//start with buffer full of zeroes
	buf_size = 22500;
	buf = (Uint8 *) malloc(sizeof(Uint8) * buf_size);
	memset(buf, 0, buf_size);
	buf_start = 0;
	buf_end = 1024;
	buf_empty = 0;
	fillcount = buf_end - buf_start;

	screen = SDL_SetVideoMode(640, 480, 0, SDL_HWSURFACE | SDL_DOUBLEBUF); //SDL_FULLSCREEN);
	glob_bpp = screen->format->BitsPerPixel;

	SDL_WM_SetCaption(BLISS_VERSION, BLISS_VERSION);

	//audio_test();
	SDL_EnableKeyRepeat(150, 50);
	return;
}

void next_frame(double left, double right)
{
	append_buf(left);
	append_buf(right);
	return;
}

static int current_x;
static int current_y;
static int current_i1;
static int current_i2;
static int current_kmod;
static int current_type;
static SDL_Event event;

int ext_poll_event(void)
{
	while (SDL_PollEvent(&event)){
		current_type = event.type;
		if (event.type == SDL_KEYDOWN) {
			current_i1 = event.key.keysym.sym;
			current_kmod = SDL_GetModState();
			//TODO: current_x, current_y = mouse location?
			return -1;
		} else if (event.type == SDL_MOUSEBUTTONDOWN) {
			current_i1 = event.button.button;
			current_kmod = SDL_GetModState();
			current_x = event.button.x;
			current_y = event.button.y;
			return -1;
			break;
		} else if (event.type == SDL_MOUSEBUTTONUP) {
			current_i1 = event.button.button;
			current_kmod = SDL_GetModState();
			current_x = event.button.x;
			current_y = event.button.y;
			return -1;
			break;
		}
	}
	return 0;
}

int export_current_i1(void)
{
	return current_i1;
}

int export_current_i2(void)
{
	return current_i2;
}

int export_current_kmod(void)
{
	return current_kmod;
}

int export_current_x(void)
{
	return current_x;
}

int export_current_y(void)
{
	return current_y;
}

char* export_current_type(void)
{
	return current_type;
}

void ext_start_drawing(void) {
         if ( SDL_MUSTLOCK(screen) ) {
                 if ( SDL_LockSurface(screen) < 0 )
                         return;
         }
		return;
}

void ext_update_screen(void) {
         /* Update the display */
         //if ( SDL_MUSTLOCK(screen) ) {
                 //SDL_UnlockSurface(screen);
         //}
         //SDL_UpdateRect(screen, 0, 0, 0, 0);
    SDL_Flip(screen);

    return;
}

static SDL_Rect dstrect;

void ext_fill_rect(int x, int y, int w, int h, int c)
{
	dstrect.x = x;
	dstrect.y = y;
	dstrect.w = w;
	dstrect.h = h;
	SDL_FillRect(screen, &dstrect, c);
}

int export_font_height(void)
{
	return TTF_FontHeight(font);
}

int ext_buffer_used(void)
{
	int r;
	SDL_LockAudio();
	r = buf_end - buf_start;
	if (r == 0 && !buf_empty) r = buf_size;
	SDL_UnlockAudio();
	if (r < 0) r = buf_size - r;
	return r;
}

int ext_fillcount(void)
{
	return fillcount >> 1;
}

int is_kmod(int a, int b)
{
	return (a & b) != 0;
}

int wave_size;
int export_wave_size(void)
{
	return wave_size;
}

char *load_wave(char *filename)
{
	/* Converting some WAV data to hardware format */
	SDL_AudioSpec wav_spec;
	SDL_AudioCVT  wav_cvt;
	Uint32 wav_len;
	Uint8 *wav_buf;
	int ret;

	/* Load the test.wav */
	if( SDL_LoadWAV(filename, &wav_spec, &wav_buf, &wav_len) == NULL ){
	  fprintf(stderr, "Could not open %s: %s\n", filename, SDL_GetError());
	  exit(-1);
	}
                                            
	/* Build AudioCVT */
	ret = SDL_BuildAudioCVT(&wav_cvt,
			wav_spec.format, wav_spec.channels, wav_spec.freq,
			wanted.format, wanted.channels, wanted.freq);

	/* Check that the convert was built */
	if(ret==-1){
	  fprintf(stderr, "Couldn't build converter!\n");
	  SDL_CloseAudio();
	  SDL_FreeWAV(wav_buf);
	  exit(-1);
	}

	/* Setup for conversion */
	wav_cvt.buf=(Uint8 *)malloc(wav_len*wav_cvt.len_mult);
	wav_cvt.len=wav_len;
	memcpy(wav_cvt.buf, wav_buf, wav_len);

	/* We can delete to original WAV data now */
	SDL_FreeWAV(wav_buf);

	/* And now we're ready to convert */
	SDL_ConvertAudio(&wav_cvt);

	wave_size = wav_cvt.len * wav_cvt.len_ratio;
	return wav_cvt.buf;
}

void *ext_make_color(int r, int g, int b)
{
    SDL_Color *res;

    res = (SDL_Color *) malloc(sizeof(SDL_Color));
    res->r = r;
    res->g = g;
    res->b = b;

    return res;
}

int ext_convert_color(int r, int g, int b)
{
    return SDL_MapRGB(screen->format, r, g, b);
}

void ext_blit_surface(SDL_Surface *surf, int x, int y)
{
    SDL_Rect rect;

    assert(surf);

    rect.x = x;
    rect.y = y;
    SDL_BlitSurface(surf, NULL, screen, &rect);
}

void *ext_new_surface(int w, int h)
{
    SDL_Surface *res;
    SDL_PixelFormat *fmt = screen->format;

    res = SDL_CreateRGBSurface(SDL_HWSURFACE, w, h, fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask,
	    fmt->Bmask, fmt->Amask);
    //res = SDL_CreateRGBSurface(SDL_HWSURFACE, w, h, glob_bpp, rmask, gmask,
	    //bmask, amask);
    return res;
}

void ext_fill_rect_surface(SDL_Surface *surf, int x, int y, int w, int h, int c)
{
    dstrect.x = x;
    dstrect.y = y;
    dstrect.w = w;
    dstrect.h = h;
    assert(surf);
    SDL_FillRect(surf, &dstrect, c);
}

int last_mouse_x, last_mouse_y;

void ext_get_mouse_state()
{
    SDL_GetMouseState(&last_mouse_x, &last_mouse_y);
}

int get_last_mouse_x()
{
    return last_mouse_x;
}

int get_last_mouse_y()
{
    return last_mouse_y;
}

void ext_blit_img_to(SDL_Surface *src, SDL_Surface *dst, int x, int y)
{
    SDL_Rect rect;
    int status;

    assert(src);
    assert(dst);

    rect.x = x;
    rect.y = y;
    status = SDL_BlitSurface(src, NULL, dst, &rect);
    if (status) {
	fprintf(stderr, "blit error: %d\n", status);
    }
}

void ext_blit_img(SDL_Surface *image, int x, int y)
{
    assert(image);

    //boxColor(screen, x, y, x + image->w - 1, y + image->h - 1, 255 * 256 + 255);
    ext_blit_img_to(image, screen, x, y);
}

void partial_blit_img(void *image, int rx, int ry, int rw, int rh, int x, int y)
{
    SDL_Rect src;
    SDL_Rect dest;

    assert(image);

    src.x = rx;
    src.y = ry;
    src.w = rw;
    src.h = rh;
    dest.x = x;
    dest.y = y;
    SDL_BlitSurface((SDL_Surface *) image, &src, screen, &dest);
}

void *free_img(void *image)
{
    assert(image);

    SDL_FreeSurface((SDL_Surface *) image);
    return NULL;
}

SDL_Surface *render_text(char *s)//, TTF_Font *font)
{
    assert(font);
    assert(s);

    return TTF_RenderText_Solid(font, s, white);
}

void *free_ttf_font(TTF_Font *font)
{
    assert(font);

    TTF_CloseFont(font);
    return NULL;
}

int get_img_width(SDL_Surface *i)
{
    assert(i);
    return i->w;
}

int get_img_height(SDL_Surface *i)
{
    assert(i);
    return i->h;
}

void img_set_alpha(SDL_Surface *i, int a)
{
    assert(i);

    SDL_SetAlpha(i, SDL_SRCALPHA, a);
}

SDL_Surface *draw_arrow_kludge()
{
    Sint16 x[3],  y[3];
    SDL_Surface *res;

    res = ext_new_surface(21, 21);


    filledCircleRGBA(res, 10, 10, 10, 255, 255, 255, 255);
    filledCircleRGBA(res, 10, 10, 8, 0, 128, 0, 255);
    x[0] = 19;
    y[0] = 10;
    x[1] = 4;
    y[1] = 4;
    x[2] = 4;
    y[2] = 16;
    dstrect.x = 0;
    dstrect.y = 0;
    dstrect.w = 16;
    dstrect.h = 16;
    filledPolygonColor(res, x, y, 3, 0x80FF80FF);

    return res;
}

void ext_box(SDL_Surface *img, int x, int y, int x2, int y2, int c)
{
    boxColor(img, x, y, x2, y2, c);
}

void ext_set_colorkey(SDL_Surface *img, int c)
{
    SDL_SetColorKey(img, SDL_SRCCOLORKEY, c);
}

double ext_int_to_float(int i)
{
    float f;
    f = *((float *) (&i));

    return f;
}

FILE *wrap_fopen(char *f, char *m)
{
    return fopen(f, m);
}

void wrap_fclose(FILE *fp)
{
    fclose(fp);
}

int wrap_feof(FILE *fp)
{
    return feof(fp);
}

char wrap_fgetc(FILE *fp)
{
    return fgetc(fp);
}
