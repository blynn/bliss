#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <SDL.h>

#include <assert.h>

#include "audio.h"
#include "colour.h"
#include "voice.h"
#include "midi.h"

#include "textbox.h"
#include "menu.h"
#include "button.h"
#include "checkbox.h"
#include "window.h"
#include "bliss.h"

#include "about.h"
#include "file_window.h"
#include "gui.h"
#include "compan.h"

#include "config.h"

#include "file.h"

extern gen_info_t out_info;
extern gen_info_t dummy_info;

#include "version.h"

int navtype;
void *navthing;
node_ptr navoutnode;

ins_ptr current_ins;

static SDL_Surface *screen;

enum {
    minroot_w = 600,
    minroot_h = 400,
    defroot_w = 800,
    defroot_h = 600,
};

enum {
    state_normal,
    state_quit
};

static int state;

extern widget_t aux_rect;

static widget_t root;
static widget_t info_rect;
static widget_t navbar;

static menubar_t menubar;

label_t navlocation;
button_ptr navupb;

window_t about_window;
file_window_t file_window;

static darray_ptr command_list, inseditlist, orcheditlist;
//TODO: move cancellist and friends to canvas.c?
static darray_ptr cancellist;

static void put_size(int w, int h)
{
    menubar->w->localx = 0;
    menubar->w->localy = 0;
    menubar->w->w = w;

    navbar->localx = 0;
    navbar->localy = menubar->w->h;
    navbar->w = w;
    navbar->h = 32;

    info_rect->w = w;
    info_rect->h = 32;
    info_rect->localx = 0;
    info_rect->localy = menubar->w->h + navbar->h;

    compan->w = 5 * 40 - 4 + 16;
    compan->h = 144;
    compan->localx = 0;
    compan->localy = h - compan->h;

    canvas->localx = compan->w;
    canvas->localy = info_rect->localy + info_rect->h;
    canvas->w = w - compan->w;
    canvas->h = h - canvas->localy;

    {
	int temp;
	temp = info_rect->localy + info_rect->h;
	aux_put_geometry(0, temp, compan->w, compan->localy - temp);
    }

    widget_move_children(root);
}

static void init_libs(void)
{
    int status;
    int flag;
    char *s;
    int n;

    status = SDL_Init(SDL_INIT_EVERYTHING);
    if (status) {
	fprintf(stderr, "init: SDL_Init failed: %s\n", SDL_GetError());
	exit(-1);
    }

    //font_init();

    s = config_at("latency");
    n = s ? atoi(s) : 512;
    audio_init(n);

    SDL_WM_SetCaption("Bliss", "Bliss");
    SDL_WM_SetIcon(SDL_LoadBMP("icon.bmp"), NULL);

    flag = SDL_RESIZABLE | SDL_HWSURFACE | SDL_DOUBLEBUF;
    screen = SDL_SetVideoMode(defroot_w, defroot_h, 0, flag);
    widget_set_screen(screen);

    colour_init(screen->format);

    /*
    {
	static unsigned char fnt[2304];
	FILE *fp;
	fp = fopen("6x9.fnt", "rb");
	fread(fnt, 1, 2304, fp);
	fclose(fp);
	gfxPrimitivesSetFont(fnt, 6, 9);
    }
    */
    return;
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

void gui_quit()
{
    state = state_quit;
}

static void new_unit(void *data)
{
    canvas_place_unit_start(canvas, (uentry_ptr) data);
    compan_push(compan, cancellist);
}

static void cancelbuttoncb(void *data)
{
    canvas_placement_finish(canvas);
    compan_pop(compan);
    //TODO: remove "Place Unit" text
}

static void newinscb(void *data)
{
    canvas_place_ins_start(canvas);
    compan_push(compan, cancellist);
}

static void newvoicecb(void *data)
{
    canvas_place_voice_start(canvas);
    compan_push(compan, cancellist);
}

static checkbox_t keycheckbox;
static label_t keycheckboxl;
static button_t recordb, playb;

static int is_recording, last_tick, last_playtick, is_playing;

static void info_rect_update(widget_ptr w)
{
    widget_raised_background(info_rect);
    widget_draw_children(info_rect);

    widget_filled_circle(info_rect, 175, 15, 8, c_darkshadow);
}

static void bliss_clear()
{
    SDL_LockAudio();
    //TODO: stop playing track
    orch_clear(orch);
    SDL_UnlockAudio();
}

static void back_to_orch(void *data) {
    gui_edit_orch();
}

static void back_to_ins(void *data) {
    gui_edit_ins((ins_ptr) data);
}

static void (* go_back_fn)(void *data);
static void *go_back_data;

void gui_back()
{
    go_back_fn(go_back_data);
}

void gui_edit_orch()
{
    navtype = nav_orch;
    navthing = orch;
    navoutnode = NULL;
    widget_hide_all(navbar);
    canvas_put_graph(canvas, orch->graph);
    canvas_select_nothing(canvas);
    compan_put(compan, orcheditlist);
    label_put_text(navlocation, "Orchestra View");
    widget_show(navlocation->w);
    widget_update(navbar);
    widget_update(canvas);
    widget_update(compan);
    request_update(root);
}

void gui_edit_ins(ins_ptr ins)
{
    navtype = nav_ins;
    navthing = ins;
    navoutnode = ins->out;
    widget_hide_all(navbar);
    go_back_data = orch;
    go_back_fn = back_to_orch;
    widget_show(navupb->w);
    canvas_put_graph(canvas, ins->graph);
    canvas_select_nothing(canvas);
    compan_put(compan, inseditlist);
    label_put_text(navlocation, "Instrument View");
    widget_show(navlocation->w);
    widget_update(navbar);
    widget_update(canvas);
    widget_update(compan);
    request_update(root);
}

void gui_edit_voice(voice_ptr voice)
{
    navtype = nav_voice;
    navthing = voice;
    navoutnode = voice->out;
    widget_hide_all(navbar);
    go_back_data = current_ins;
    go_back_fn = back_to_ins;
    widget_show(navupb->w);
    compan_put(compan, command_list);
    canvas_put_graph(canvas, voice->graph);
    canvas_select_nothing(canvas);
    label_put_text(navlocation, "Voice View");
    widget_show(navlocation->w);
    widget_update(navbar);
    widget_update(canvas);
    widget_update(compan);
    request_update(root);
}

static void file_new()
{
    voice_ptr voice;
    ins_ptr ins;
    node_ptr n0, n1;

    orch_init(orch);

    n0 = orch_add_ins(orch, "ins0", canvas->w / 2, canvas->h / 2);
    ins = ((node_data_ptr) n0->data)->ins;
    current_ins = ins;

    n1 = add_ins_unit("out", out_uentry, ins, canvas->w - 100, canvas->h / 2);
    ins->out = n1;

    n0 = add_voice("voice0", ins, 5, canvas->h / 2);
    voice = node_get_voice(n0);

    voice->out = add_voice_unit("out", out_uentry, voice, canvas->w - 100, canvas->h / 2);
    add_voice_unit("freq", utable_at("dummy"), voice, 5, canvas->h / 2);

    add_edge(ins->graph, n0, n1, 0);

    gui_edit_voice(voice);
}

static void loadcb(char *filename)
{
    audio_stop();
    bliss_clear();
    orch_init(orch);
    file_load(filename, orch);
    current_ins = NULL;
    gui_edit_orch();
    widget_update(canvas);
    widget_update(aux_rect);
    audio_start();
}

static void savecb(char *filename)
{
    file_save(filename, orch);
}

static void savemenuitemcb(void *data)
{
    file_window_setup(file_window, "Save File", "Save", savecb);
    window_open(file_window->win);
}

static void newmenuitemcb(void *data)
{
    bliss_clear();
    file_new();
    widget_update(root);
    request_update(root);
}

static double ticker()
{
    return orch_tick(orch);
}

static void do_event(ins_ptr ins, event_ptr e)
{
    switch(e->type) {
	case ev_noteon:
	    //midi_note_on(ins, e->x1, e->x2);
	    ins_note_on(ins, e->x1, ((double) e->x2) / 127.0);
	    break;
	case ev_noteoff:
	    //midi_note_off(ins, e->x1);
	    ins_note_off(ins, e->x1);
	    break;
    }
}

int compute_whole_song(int samprate, void (*outcb)(double))
{
    int t = 0;
    int i;

    play_state_rewind();
    while (!play_state_finished()) {
	int t1;
	t1 = play_state_delta() * samprate / 1000;
	for (i=0; i<t1; i++) {
	    outcb(ticker());
	    t++;
	}
	play_state_advance(do_event);
    } while (!play_state_finished());

    for (i=2*samprate; i; i--) {
	outcb(ticker());
	t++;
    }
    return t;
}

static void rendermenuitemcb(void *data)
{
    FILE *fp;
    int samples;
    int wavsamprate = 44100;

    void write2(int n) {
	int m = n;
	fputc(m & 255, fp);
	m >>= 8;
	fputc(m & 255, fp);
    }

    void write4(int n) {
	int m = n;
	fputc(m & 255, fp);
	m >>= 8;
	fputc(m & 255, fp);
	m >>= 8;
	fputc(m & 255, fp);
	m >>= 8;
	fputc(m & 255, fp);
    }

    void outdouble(double d)
    {
	int n = d * 4096;
	write2(n);
	write2(n);
    }

    fp = fopen("bliss.wav", "wb");
    fprintf(fp, "RIFF");
    fprintf(fp, "????");
    fprintf(fp, "WAVE");
    fprintf(fp, "fmt ");
    write4(16);
    write2(1);
    write2(2);
    write4(wavsamprate);
    write4(4 * wavsamprate);
    write2(4);
    write2(16);
    fprintf(fp, "data");
    fprintf(fp, "????");

    audio_stop();
    samples = compute_whole_song(wavsamprate, outdouble);

    fseek(fp, 4L, SEEK_SET);
    write4(samples * 4 + 36);
    fseek(fp, 40L, SEEK_SET);
    write4(samples * 4);

    audio_start();
    fclose(fp);
}

static void loadmenuitemcb(void *data)
{
    file_window_setup(file_window, "Open File", "Open", loadcb);
    window_open(file_window->win);
}

static void quitcb(void *data)
{
    gui_quit();
}

static void aboutcb(void *data)
{
    window_open(about_window);
}

static void init_menu()
{
    menu_ptr m;

    menubar_init(menubar, root);
    widget_show(menubar->w);

    m = menubar_add_button(menubar, "File");
    menu_add_command(m, "Open...", loadmenuitemcb, NULL);
    menu_add_command(m, "Save...", savemenuitemcb, NULL);
    menu_add_command(m, "New", newmenuitemcb, NULL);
    menu_add_command(m, "Render", rendermenuitemcb, NULL);
    menu_add_command(m, "Quit", quitcb, NULL);

    m = menubar_add_button(menubar, "Help");
    menu_add_command(m, "About...", aboutcb, NULL);
}

static track_ptr newtrack;

static inline void add_event(int type, int x1, int x2)
{
    int t;

    t = SDL_GetTicks();
    track_add_event(newtrack, t - last_tick, type, x1, x2);
    last_tick = t;
}

static void stop_recording()
{
    is_recording = 0;
    widget_filled_circle(info_rect, 175, 15, 8, c_darkshadow);
    //TODO: only need to update the LED
    request_update(info_rect);
    //TODO: shouldn't fool with track internals like this, write wrappers
    darray_copy(current_ins->track->event, newtrack->event);
    darray_clear(newtrack->event);
    free(newtrack);
}

static void midi_note_off(ins_ptr ins, int noteno)
{
    assert(ins);
    ins_note_off(ins, noteno);
    if (is_recording && ins == current_ins) add_event(ev_noteoff, noteno, 0);
}

static void midi_note_on(ins_ptr ins, int noteno, int vel)
{
    assert(ins);
    SDL_LockAudio();
    ins_note_on(ins, noteno, ((double) vel) / 127.0);
    SDL_UnlockAudio();
    if (is_recording && ins == current_ins) add_event(ev_noteon, noteno, vel);
}

static Uint32 play_thread(Uint32 ignore, void *unused)
{
    int t, error, corrected_delta;
    if (!is_playing) return 0;

    do {
	t = SDL_GetTicks();
	error = (t - last_playtick) - play_state_delta();
	last_playtick = t;
	play_state_advance(do_event);
	if (play_state_finished()) {
	    is_playing = 0;
	    return 0;
	}
	corrected_delta = play_state_delta() - error;
    } while (corrected_delta <= 0);
    return corrected_delta;
}

static void start_playing()
{
    play_state_rewind();
    if (!play_state_finished()) {
	is_playing = 1;
	last_playtick = SDL_GetTicks();
	SDL_AddTimer(play_state_delta(), play_thread, NULL);
    }
}

static void start_recording()
{
    newtrack = (track_ptr) malloc(sizeof(track_t));
    track_init(newtrack, NULL);
    assert(current_ins);
    track_remove_all(current_ins->track);
    is_recording = 1;
    widget_filled_circle(info_rect, 175, 15, 8, c_led);
    request_update(info_rect);
    start_playing();
    last_tick = SDL_GetTicks();
}

static void toggle_recordcb(void *data)
{
    if (is_playing) {
	printf("can't record while playing\n");
	return;
    }
    if (is_recording) {
	stop_recording();
    } else {
	if (current_ins) start_recording();
    }
}

static void toggle_playcb(void *data)
{
    if (is_recording) {
	stop_recording();
	return;
    }
    if (is_playing) {
	is_playing = 0;
	//TODO: all notes off
    } else {
	start_playing();
    }
}

static int keyboardplayflag = 0;

static int symtonote[256];

static void noteoncb(int note, int vel)
{
    if (current_ins) midi_note_on(current_ins, note, vel);
}

static void noteoffcb(int note)
{
    if (current_ins) midi_note_off(current_ins, note);
}

static void specialkeyup(int sym, int mod)
{
    int n = symtonote[sym];
    if (n >=0) noteoffcb(n + 60);
}

static int keyboard_midi_on(widget_ptr w, int sym, int mod, void *data)
{
    int n;
    if (!(sym >=0 && sym < 128)) return 1;
    n = symtonote[sym];
    if (n >=0) {
	noteoncb(n + 60, 64);
	return 0;
    }
    return 1;
}

static void keyboardcb(void *data, int state)
{
    keyboardplayflag = state;
    if (keyboardplayflag) {
	//TODO: checkbox who called should also be sent?
	widget_push_keydowncb(NULL, keyboard_midi_on, NULL);
    } else {
	widget_pop_keydowncb();
    }
}

static void init_info()
{
    widget_init(info_rect, root);

    button_init(playb, info_rect);
    playb->w->localx = 5;
    playb->w->localy = 10;
    button_make_text_image(playb, "Play/Stop");
    widget_show(playb->w);
    playb->callback = toggle_playcb;

    button_init(recordb, info_rect);
    recordb->w->localx = 100;
    recordb->w->localy = 10;
    button_make_text_image(recordb, "Record");
    widget_show(recordb->w);
    recordb->callback = toggle_recordcb;

    checkbox_init(keycheckbox, info_rect);
    keycheckbox->w->localx = 200;
    keycheckbox->w->localy = 10;
    keycheckbox->state = 0;
    keycheckbox->callback = keyboardcb;
    widget_show(keycheckbox->w);

    label_init(keycheckboxl, info_rect);
    keycheckboxl->w->localx = 220;
    keycheckboxl->w->localy = 10;
    keycheckboxl->text = "Keyboard";
    widget_show(keycheckboxl->w);

    widget_show(info_rect);
    info_rect->update = info_rect_update;
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

static void main_loop(void)
{
    int lastmousex, lastmousey;
    int newx = 0, newy = 0; //to get rid of compiler warning
    int motiontimer = 0;

    Uint32 motioncb(Uint32 interval, void *data)
    {
	motion_notify();
	return 0;
    }

    SDL_GetMouseState(&lastmousex, &lastmousey);
    while (state != state_quit) {
	SDL_Event event_;
	SDL_Event *event = &event_;

	//TODO: error check
	SDL_WaitEvent(event);

	switch (event->type) {
	    case SDL_USEREVENT:
		switch(event->user.code) {
		    case code_motion:
			motiontimer = 0;
			SDL_GetMouseState(&newx, &newy);
			root_mouse_motion(lastmousex, lastmousey, newx, newy);
			lastmousex = newx;
			lastmousey = newy;
			break;
		    case code_interrupt:
			gui_quit();
			break;
		}
		break;
	    case SDL_QUIT:
		gui_quit();
		break;
	    case SDL_VIDEORESIZE:
		main_resize(event->resize.w, event->resize.h);
		widget_update(root);
		request_update(root);
		break;
	    case SDL_MOUSEBUTTONDOWN:
		/*
		if (!displaywindow) {
		    w = root;
		} else {
		    w = displaywindow->w;
		}
		*/
		root_button_down(root, event->button.button,
			event->button.x, event->button.y);
		break;
	    case SDL_MOUSEBUTTONUP:
		if (event->button.button != 1) break;

		root_button_up(root, event->button.button,
			event->button.x, event->button.y);
		break;
	    case SDL_KEYDOWN:
		root_key_down(event->key.keysym.sym,
			event->key.keysym.mod);
		break;
	    case SDL_KEYUP:
		if (keyboardplayflag) {
		    specialkeyup(event->key.keysym.sym,
			    event->key.keysym.mod);
		}
		break;
	    case SDL_MOUSEMOTION:
		if (!motiontimer) {
		    motiontimer = 1;
		    SDL_AddTimer(10, motioncb, NULL);
		}
		break;
	    default:
		break;
	}
    }
}

SDL_Event event_interrupt;

static void interrupt(int i)
{
    SDL_PushEvent(&event_interrupt);
}

static void navbar_update(widget_ptr w)
{
    widget_raised_background(w);
    widget_draw_children(w);
}

static void navupcb(void *unused)
{
    gui_back();
}

static void init_navbar()
{
    widget_init(navbar, root);
    navbar->update = navbar_update;
    navupb = button_new(navbar);
    navupb->img = SDL_LoadBMP("up.bmp");
    button_put_callback(navupb, navupcb, NULL);
    label_init(navlocation, navbar);
    widget_put_location(navlocation->w, 64, 15);
    widget_put_geometry(navupb->w, 10, 6, 20, 20);
    widget_show(navbar);
}

static void buttonmenubackcb(void *data)
{
    compan_pop(compan);
}

static void openbuttonmenucb(void *data)
{
    compan_push(compan, (darray_ptr) data);
}

static void init_command()
{
    button_ptr b;
    darray_ptr l;
    image_ptr imgcancel = SDL_LoadBMP("cancel.bmp");

    void add_unit_button(darray_ptr target_list, int row, int col, char *id)
    {
	uentry_ptr p = utable_at(id);
	b = compan_new_button(compan, row, col);
	b->img = p->img;
	b->text = p->info->name;
	b->callback = new_unit;
	b->data = (void *) p;
	darray_append(target_list, b);
    }

    darray_ptr new_list_button(darray_ptr target_list, int row, int col,
	    char *text, image_ptr image) {
	darray_ptr res = darray_new();
	b = compan_new_button(compan, 2, 4);
	b->img = imgcancel;
	b->text = "Cancel";
	b->callback = buttonmenubackcb;
	b->data = NULL;
	darray_append(res, b);

	b = compan_new_button(compan, row, col);
	b->img = image;
	b->text = text;
	b->callback = openbuttonmenucb;
	b->data = (void *) res;
	darray_append(target_list, b);
	return res;
    }

    void add_button(darray_ptr target_list, int row, int col,
	    char *text, image_ptr image,
	    void (*callback)(void *), void *data) {
	b = compan_new_button(compan, row, col);
	b->text = text;
	b->img = image;
	b->callback = callback;
	b->data = data;
	darray_append(target_list, b);
    }

    compan_init(compan, root);

    command_list = darray_new();
    inseditlist = darray_new();
    orcheditlist = darray_new();

    l = new_list_button(command_list, 0, 0, "Oscillators", SDL_LoadBMP("sine.bmp"));
    add_unit_button(l, 0, 0, "osc");
    add_unit_button(l, 0, 1, "noise");
    add_unit_button(l, 0, 2, "bloprandomwave");
    add_unit_button(l, 0, 3, "stomperosc");
    add_unit_button(l, 1, 0, "shepard");


    l = new_list_button(command_list, 0, 1, "Envelopes", SDL_LoadBMP("adsr.bmp"));
    add_unit_button(l, 0, 0, "adsr");
    add_unit_button(l, 0, 1, "stomperenv");

    l = new_list_button(command_list, 0, 2, "Filters", SDL_LoadBMP("lpf.bmp"));
    add_unit_button(l, 0, 0, "butterlpf");
    add_unit_button(l, 0, 1, "butterhpf");
    add_unit_button(l, 1, 0, "blop4plpf");
    add_unit_button(l, 2, 0, "onezero");
    add_unit_button(l, 2, 1, "onepole");
    add_unit_button(l, 2, 2, "twopole");

    add_unit_button(command_list, 1, 0, "funk2");
    add_unit_button(command_list, 1, 1, "seg");
    add_unit_button(command_list, 1, 2, "clipper");

    add_unit_button(command_list, 1, 3, "delay");

    cancellist = darray_new();
    add_button(cancellist, 2, 4, "Cancel", imgcancel,
	    cancelbuttoncb, NULL);

    add_button(inseditlist, 2, 0, "New Voice", SDL_LoadBMP("voice.bmp"),
		    newvoicecb, NULL);
    darray_append(inseditlist, command_list->item[0]);
    darray_append(inseditlist, command_list->item[1]);
    darray_append(inseditlist, command_list->item[2]);
    darray_append(inseditlist, command_list->item[3]);
    darray_append(inseditlist, command_list->item[4]);
    darray_append(inseditlist, command_list->item[5]);
    darray_append(inseditlist, command_list->item[6]);

    add_button(orcheditlist, 2, 0, "New Instrument", SDL_LoadBMP("voice.bmp"),
		    newinscb, NULL);
}

static int bliss_keydown_cb(widget_ptr w, int sym, int mod, void *data)
{
    if (sym == SDLK_F12) {
	SDL_SaveBMP(screen, "screenshot.bmp");
	printf("screenshot saved to screenshot.bmp\n");
    }
    return 0;
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

    init_command();
    init_menu();
    aux_init(root);
    init_navbar();
    init_info();
    canvas_init(canvas, root);

    about_init(about_window, root);
    file_window_init(file_window, root);

    put_size(root->w, root->h);
}

int main(int argc, char **argv)
{
    struct midi_cb_s midicbp = {
	noteoncb,
	noteoffcb,
    };

    {
	int i;
	unsigned char *s = "zsxdcvgbhnjm,l.;/";
	unsigned char *s2 = "q2w3er5t6y7ui9o0p[=]";
	for (i=0; i<256; i++) {
	    symtonote[i] = -1;
	}
	for (i=0; i<strlen(s); i++) {
	    symtonote[s[i]] = i;
	}
	for (i=0; i<strlen(s2); i++) {
	    symtonote[s2[i]] = i + 12;
	}
    }

    event_interrupt.type = SDL_USEREVENT;
    event_interrupt.user.code = code_interrupt;

    config_init();

    init_libs();

    signal(SIGINT, interrupt);
    signal(SIGTERM, interrupt);

    utable_init();

    widget_system_init();

    init_root();

    file_new();

    widget_push_keydowncb(NULL, bliss_keydown_cb, NULL);

    audio_set_ticker(ticker);
    state = state_normal;
    midi_start(&midicbp);
    widget_update(root);
    request_update(root);
    audio_start();
    main_loop();

    midi_stop();
    audio_stop();
    
    //TODO: free everything
    utable_clear();
    SDL_FreeSurface(screen);
    SDL_Quit();
    return 0;
}
