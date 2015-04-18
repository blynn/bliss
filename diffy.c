//digital filter designer
#include <SDL.h>
#include <signal.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>

#include "widget.h"
#include "textbox.h"
#include "button.h"
#include "colour.h"

enum {
    minroot_w = 600,
    minroot_h = 400,
    defroot_w = 800,
    defroot_h = 600,
};

static int interrupted = 0;

enum {
    state_normal,
    state_quit
};

static int state;

static widget_t root;
static widget_t compan;
static widget_t canvas;
static widget_t info;

static SDL_Surface *screen;

static darray_t zero_list, pole_list;

static darray_ptr command_list, cancellist;
static darray_ptr pzcomlist;

static void compan_update(widget_ptr compan)
{
    int i;
    widget_raised_background(compan);
    for (i=0; i<command_list->count; i++) {
	button_ptr p = (button_ptr) command_list->item[i];
	widget_ptr w = p->w;
	widget_update(w);
    }
}

static void compan_mouse_move(widget_ptr compan, int x, int y)
{
    int i;
    widget_raised_background(compan);
    for (i=0; i<command_list->count; i++) {
	button_ptr p = (button_ptr) command_list->item[i];
	widget_ptr w = p->w;
	if (local_contains(w, x, y)) {
	    widget_rectangle(w, -1, -1,  w->w, w->h, c_select);
	    widget_string(compan, 10, compan->h - 12, p->text, c_text);
	}
	widget_update(w);
    }
}

static void compan_handle_mousebuttondown(widget_ptr w,
	int button, int x, int y)
{
    int i;
    for (i=0; i<command_list->count; i++) {
	button_ptr p = (button_ptr) command_list->item[i];
	if (local_contains(p->w, x, y)) {
	    p->w->handle_mousebuttondown(p->w,
		    button,
		    x - p->w->localx, y - p->w->localy);
	    return;
	}
    }
}

static button_ptr new_command_button(int row, int col)
{
    button_ptr b;

    b = button_new(compan);

    widget_ptr w = b->w;
    w->localx = col * (32 + 4 + 4) + 8;
    w->localy = row * (32 + 4 + 4) + 8;
    w->w = 32 + 4;
    w->h = 32 + 4;

    return b;
}

enum {
    place_nothing = 0,
    place_zero,
    place_pole
};

static struct {
    int type;
    image_ptr img, old;
    rect_t r;
    int flag;
} canvas_placement;

static void put_command_list(darray_ptr a)
{
    command_list = a;
    widget_update(compan);
}

void newpzcb(void *data)
{
    canvas_placement.type = (int) data;
    put_command_list(cancellist);
}

static void canvas_finish_placement()
{
    canvas_placement.type = place_nothing;
    widget_update(canvas);
}

static void cancelcb(void *data)
{
    canvas_finish_placement();
    put_command_list(pzcomlist);
}

static void init_command()
{
    button_ptr b;

    widget_init(compan, root);
    widget_show(compan);
    compan->update = compan_update;
    compan->handle_mousebuttondown = compan_handle_mousebuttondown;

    pzcomlist = darray_new();
    b = new_command_button(0, 0);
    b->text = "Zero";
    b->img = NULL;
    b->callback = newpzcb;
    b->data = (void *) place_zero;
    darray_append(pzcomlist, b);

    b = new_command_button(0, 1);
    b->text = "Pole";
    b->img = NULL;
    b->callback = newpzcb;
    b->data = (void *) place_pole;
    darray_append(pzcomlist, b);

    command_list = pzcomlist;

    cancellist = darray_new();
    b = new_command_button(1, 4);
    b->text = "Cancel";
    b->img = SDL_LoadBMP("cancel.bmp");
    b->callback = cancelcb;
    b->data = NULL;
    darray_append(cancellist, b);
}

struct complex_s {
    double re, im;
};
typedef struct complex_s complex_t[1];
typedef struct complex_s *complex_ptr;

void complex_init(complex_ptr z)
{
    z->re = 0.0;
    z->im = 0.0;
}

void complex_clear(complex_ptr z)
{
}

complex_ptr complex_new()
{
    complex_ptr res = (complex_ptr) malloc(sizeof(complex_t));
    complex_init(res);
    return res;
}

void complex_free(complex_ptr z)
{
    free(z);
}

struct zeropole_s {
    int type;
    complex_t z;
};

static int canvas_range, canvas_origin_x, canvas_origin_y;
static int canvas_r;

static void canvas_update(widget_ptr w)
{
    int i;

    widget_box_rect(canvas, c_canvas);
    //draw axes
    widget_line(w, canvas_origin_x - canvas_range, canvas_origin_y,
	    canvas_origin_x + canvas_range, canvas_origin_y, c_highlight);
    widget_line(w, canvas_origin_x, canvas_origin_y - canvas_range,
	    canvas_origin_x, canvas_origin_y + canvas_range, c_highlight);
    //draw unit circle
    widget_circle(w, canvas_origin_x, canvas_origin_y,
	    canvas_r, c_highlight);

    for (i=0; i<zero_list->count; i++) {
	complex *z = (complex *) zero_list->item[i];
	int x, y;
	x = canvas_r * creal(*z) + canvas_origin_x;
	y = canvas_r * cimag(*z) + canvas_origin_y;
	widget_circle(w, x, y, 5, c_highlight);
    }

    for (i=0; i<pole_list->count; i++) {
	complex *z = (complex *) pole_list->item[i];
	int x, y;
	x = canvas_r * creal(*z) + canvas_origin_x;
	y = canvas_r * cimag(*z) + canvas_origin_y;
	widget_line(w, x - 5, y - 5, x + 5, y + 5, c_highlight);
	widget_line(w, x - 5, y + 5, x + 5, y - 5, c_highlight);
    }
}

static void canvas_mouse_move(widget_ptr w, int x, int y)
{
    char s[80];
    widget_update(canvas);
    widget_clip(canvas);
    switch(canvas_placement.type) {
	case place_zero:
	    widget_filled_circle(canvas, x, y, 5, c_select);
	    break;
	case place_pole:
	    widget_box(canvas, x - 5, y - 5, x + 5, y + 5, c_select);
	    break;
    }
    widget_unclip();
    sprintf(s, "%.3f + %.3fi", 
	(double) (x - canvas_origin_x) / (double) canvas_r,
	(double) (y - canvas_origin_y) / (double) canvas_r);
    widget_string(canvas, 5, 5, s, c_highlight);
}

static void canvas_handle_mousebuttondown(widget_ptr w, int button, int x, int y)
{
    if (canvas_placement.type) {
	complex *z = (complex *) malloc(sizeof(complex));
	*z = (double) (x - canvas_origin_x) / (double) canvas_r;
	*z += 1i * (double) (y - canvas_origin_y) / (double) canvas_r;

	switch (canvas_placement.type) {
	    case place_zero:
		darray_append(zero_list, z);
		break;
	    case place_pole:
		darray_append(pole_list, z);
		break;
	    default:
		break;
	}
	canvas_finish_placement();
	widget_update(info);
	put_command_list(pzcomlist);
    }
}

static void init_canvas()
{
    widget_init(canvas, root);
    widget_show(canvas);
    canvas->update = canvas_update;
    canvas->handle_mousebuttondown = canvas_handle_mousebuttondown;
}

static void info_update(widget_ptr w)
{
    int i;
    int max = 180;
    complex z;
    double g, gd;
    widget_raised_background(w);
    widget_line(w, 10, w->h - 10, 10 + max, w->h - 10, c_text);
    widget_line(w, 10, w->h - 10, 10, w->h - 10 - 180, c_text);
    for (i=0; i<=max; i++) {
	int j;
	double omega = M_PI * (double) i / (double) max;
	z = cos(omega) + sin(omega) * 1i;
	g = 1;
	gd = 1;
	for (j=0; j<zero_list->count; j++) {
	    g *= cabs(z - *((complex *) zero_list->item[j]));
	}
	for (j=0; j<pole_list->count; j++) {
	    gd *= cabs(z - *((complex *) pole_list->item[j]));
	}
	g /= gd;
	widget_pixel(w, 10 + i, w->h - 10 - g * 64, c_text);
    }
}

static void init_info()
{
    widget_init(info, root);
    widget_show(info);
    info->update = info_update;
}

static void put_size(int w, int h)
{
    compan->w = 5 * 40 - 4 + 16;
    compan->h = 144;
    compan->localx = 0;
    compan->localy = h - compan->h;

    info->w = compan->w;
    info->h = 200;
    info->localx = 0;
    info->localy = compan->localy - info->h;

    canvas->localx = compan->w;
    canvas->localy = 0;
    canvas->w = w - compan->w;
    canvas->h = h;
    canvas_range = canvas->h;
    if (canvas_range > canvas->w) canvas_range = canvas->w;
    canvas_range /= 2;
    canvas_origin_x = canvas->w / 2;
    canvas_origin_y = canvas->h / 2;
    canvas_r = canvas_range / 2;

    widget_move_children(root);
}

static void main_resize(int x, int y)
{
    int flag;

    if (x < minroot_w) x = minroot_w;
    if (y < minroot_h) y = minroot_h;
    SDL_FreeSurface(screen);
    flag = SDL_RESIZABLE | SDL_HWSURFACE | SDL_DOUBLEBUF; //SDL_FULLSCREEN);
    screen = SDL_SetVideoMode(x, y, 0, flag);
    widget_set_screen(screen);
    root->w = x; root->h = y;
    put_size(x, y);
}

static void root_mouse_move(widget_ptr w, int x0, int y0, int x1, int y1)
{
    if (local_contains(compan, x1, y1) || local_contains(compan, x0, y0)) {
	compan_mouse_move(compan, x1 - compan->globalx, y1 - compan->globaly);
    } else if (canvas_placement.type && state == state_normal) {
	canvas_mouse_move(canvas, x1 - canvas->globalx, y1 - canvas->globaly);
    }
}

static void main_loop(void)
{
    int lastmousex, lastmousey;
    SDL_GetMouseState(&lastmousex, &lastmousey);
    while (state != state_quit && !interrupted) {
	widget_ptr w = root;
	SDL_Event event_;
	SDL_Event *event = &event_;

	while (SDL_PollEvent(event)) switch (event->type) {
	    case SDL_MOUSEMOTION:
		root_mouse_move(root, lastmousex, lastmousey,
		    event->motion.x, event->motion.y);
		lastmousex = event->motion.x;
		lastmousey = event->motion.y;

		break;
	    case SDL_QUIT:
		state = state_quit;
		break;
	    case SDL_VIDEORESIZE:
		main_resize(event->resize.w, event->resize.h);
		root->update(root);
		break;
	    case SDL_MOUSEBUTTONDOWN:
		if (local_contains(w, event->button.x, event->button.y)) {
		    w->handle_mousebuttondown(w, event->button.button,
			    event->button.x - w->globalx, event->button.y - w->globaly);
		}
		break;
	    case SDL_MOUSEBUTTONUP:
		root_button_up(root, event->button.button,
			event->button.x, event->button.y);
		break;
	    case SDL_KEYDOWN:
		root_key_down(event->key.keysym.sym,
			event->key.keysym.mod);
		break;
	    default:
		break;
	}
	SDL_Flip(screen);
	SDL_Delay(10);
    }
}

static void root_update()
{
    int i;
    widget_ptr w = root;
    SDL_FillRect(screen, NULL, 0);
    for (i=0; i<w->show_list->count; i++) {
	widget_ptr p = w->show_list->item[i];
	p->update(p);
    }
}

static void init_root()
{
    root->w = defroot_w;
    root->h = defroot_h;
    root->localx = 0;
    root->localy = 0;
    root->globalx = 0;
    root->globaly = 0;
    root->handle_mousebuttondown = root_button_down;
    root->update = root_update;
}

static void init_libs(void)
{
    int status;
    int flag;

    status = SDL_Init(SDL_INIT_EVERYTHING);
    if (status) {
	fprintf(stderr, "init: SDL_Init failed: %s\n", SDL_GetError());
	exit(-1);
    }
    atexit(SDL_Quit);

    SDL_WM_SetCaption("Digital Filter Designer", "Digital Filter Designer");

    flag = SDL_RESIZABLE | SDL_HWSURFACE | SDL_DOUBLEBUF;// | SDL_FULLSCREEN;
    screen = SDL_SetVideoMode(defroot_w, defroot_h, 0, flag);
    widget_set_screen(screen);

    colour_init(screen->format);

    SDL_EnableKeyRepeat(150, 50);

    return;
}

int main(int argc, char **argv)
{
    void interrupt(int i) { interrupted = 1; }
    signal(SIGINT, interrupt);
    signal(SIGTERM, interrupt);

    init_libs();

    darray_init(zero_list);
    darray_init(pole_list);

    init_root();
    init_command();
    //init_aux();
    init_canvas();
    init_info();

    put_size(root->w, root->h);

    state = state_normal;
    root->update(root);

    main_loop();
    
    //TODO: free everything
    return 0;
}
