#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <signal.h>
#include "root.h"
#include "font.h"
#include "mlist.h"
#include "darray.h"
#include "audio.h"
#include "convert_buzz.h"
#include "main.h"

enum {
    minroot_w = 320,
    minroot_h = 160,
};

SDL_Surface *screen;

static int root_w = 640;
static int root_h = 480;

window_ptr active_window;
int modal_window;

static darray_t window_list;

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

    font_init();

    audio_init();

    flag = SDL_RESIZABLE | SDL_HWSURFACE | SDL_DOUBLEBUF; //SDL_FULLSCREEN);
    screen = SDL_SetVideoMode(root_w, root_h, 0, flag);

    signal(SIGINT, exit);
    signal(SIGTERM, exit);

    SDL_WM_SetCaption("Bliss", "Bliss");
    //SDL_WM_SetCaption(BLISS_VERSION, BLISS_VERSION);

    SDL_EnableKeyRepeat(150, 50);
    return;
}

widget_ptr root;

enum {
    state_normal,
    state_quit
};

static int state;

void main_quit()
{
    state = state_quit;
}

static int dragx, dragy;
static window_ptr drag_win;

void main_drag_start(window_ptr win)
{
    widget_getmousexy(root, &dragx, &dragy);
    drag_win = win;
}

void main_drag_stop()
{
    drag_win = NULL;
}

static void handle_mousebuttondown(event_ptr event)
//modal window gets the click if it exists (i.e. current active window)
//otherwise top window becomes active
{
    window_ptr win;
    if (!modal_window) {
	//new active window is top window containing mouse
	int i;
	for (i=window_list->count-1; i>=0; i--) {
	    win = (window_ptr) window_list->item[i];
	    if (((widget_ptr) win)->visible) {
		if (widget_has_mouse((widget_ptr) win)) {
		    active_window = win;
		    break;
		}
	    }
	}
    }

    //check for drag start
    win = active_window;
    if (window_titlebar_has_mouse(win)) {
	main_drag_start(win);
    } else {
	widget_handle_event((widget_ptr) active_window, event);
    }
}

static void handle_keydown(event_ptr event)
//send keystrokes to active window
{
    widget_ptr w = (widget_ptr) active_window;
    widget_handle_event(w, event);
}

static void main_resize(int x, int y)
{
    int flag;

    if (x < minroot_w) x = minroot_w;
    if (y < minroot_h) y = minroot_h;
    widget_put_size(root, x, y);
    SDL_FreeSurface(screen);
    flag = SDL_RESIZABLE | SDL_HWSURFACE | SDL_DOUBLEBUF; //SDL_FULLSCREEN);
    screen = SDL_SetVideoMode(x, y, 0, flag);
}

static void main_loop(void)
{
    int i;

    state = state_normal;
    while (state != state_quit) {
	SDL_Event event_;
	SDL_Event *event = &event_;

	SDL_LockAudio();
	audio_buffer();
	SDL_UnlockAudio();

	update_mousestate();
	update_modstate();
	if (drag_win) {
	    int x, y;
	    widget_ptr w = (widget_ptr) drag_win;
	    widget_getmousexy(root, &x, &y);
	    widget_put_local(w, w->localx + x - dragx, w->localy + y - dragy);
	    dragx = x;
	    dragy = y;
	}
	for (i=0; i<window_list->count; i++) {
	    widget_update((widget_ptr) window_list->item[i]);
	}
	SDL_Flip(screen);
	while (SDL_PollEvent(event)) switch (event->type) {
	    case SDL_QUIT:
		main_quit();
		break;
	    case SDL_MOUSEBUTTONDOWN:
		handle_mousebuttondown(event);
		break;
	    case SDL_MOUSEBUTTONUP:
		if (drag_win) {
		    main_drag_stop();
		} else {
		    widget_handle_event(root, event);
		}
		break;
	    case SDL_KEYDOWN:
		handle_keydown(event);
		break;
	    case SDL_VIDEORESIZE:
		main_resize(event->resize.w, event->resize.h);
	    default:
		widget_handle_event(root, event);
		break;
	}
	SDL_Delay(10);
    }
}

int main(int argc, char **argv)
{
    window_ptr rootwin;

    init_libs();
    init_colour(screen->format);
    mlist_init();
    load_plugin_dir(".");
    convert_buzz_init();

    darray_init(window_list);
    rootwin = root_new(root_w, root_h);
    root = (widget_ptr) rootwin;
    active_window = rootwin;

    SDL_PauseAudio(0);
    main_loop();
    SDL_PauseAudio(1);

    //TODO: free everything
    return 0;
}

void widget_focus(widget_ptr w)
{
    window_ptr win = active_window;
    if (win->focus_widget) widget_lose_focus(win->focus_widget);
    win->focus_widget = NULL;
    if (w) {
	if (w->can_focus) {
	    w->has_focus = 1;
	    win->focus_widget = w;
	}
    }

}

void main_add_window(window_ptr win)
{
    darray_append(window_list, win);
}

void main_open_modal_window(window_ptr win)
{
    modal_window = 1;
    active_window = win;
    widget_show((widget_ptr) win);
}

void main_close_window(window_ptr win)
{
    if (modal_window) modal_window = 0;
    if (win == active_window) active_window = (window_ptr) root;
    widget_hide((widget_ptr) win);
}

void main_screenshot()
{
    SDL_SaveBMP(screen, "screenshot.bmp");
}
