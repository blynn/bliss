#include "audio.h"
#include "window.h"
#include "container.h"
#include "machine_window.h"
#include "pattern_area.h"
#include "song_area.h"
#include "convert_buzz.h"
#include "menu.h"
#include "label.h"
#include "main.h"
#include "about.h"
#include "filewin.h"

enum {
    op_open,
    op_saveas,
    op_import,
};

enum {
    bar1_h = 20,
    statusbar_h = 16,
    padding_h = 5,
};

static int root_arena_w, root_arena_h;
static window_t rootwin;
static window_ptr aboutwin;
static window_ptr filewin;

static widget_ptr pattern_window, machine_window, song_window;
static widget_ptr now_showing;
static label_t statuslabel;

static song_t song;
static menubar_t mb;

static textbox_t bpmtb, tpbtb;

static int file_op;

void root_edit_song(song_ptr s)
{
    char buf[30];

    song_area_edit((song_area_ptr) song_window, song);
    machine_window_edit((machine_window_ptr) machine_window, song);
    pattern_area_edit((pattern_area_ptr) pattern_window, song);
    audio_put_song(song);
    sprintf(buf, "%d", song->bpm);
    textbox_put_text(bpmtb, buf);
    sprintf(buf, "%d", song->tpb);
    textbox_put_text(tpbtb, buf);
}

void root_put_buzz_coord(machine_ptr m, double x, double y)
{
    machine_window_put_buzz_coord((machine_window_ptr) machine_window, m, x, y);
}

void root_pattern_check(machine_ptr m)
    //m is about to be deleted
    //if editing a pattern belonging to m, change to NULL
{
    pattern_area_ptr pa = (pattern_area_ptr) pattern_window;
    if (pa->pattern && pa->pattern->machine == m) {
	pattern_area_put_machine(pa, song->master);
    }
}

void root_edit_pattern(pattern_ptr p)
{
    pattern_area_put_pattern((pattern_area_ptr) pattern_window, p);
}

void root_resize(widget_ptr w, void *data)
{
    int root_w, root_h;

    root_w = w->w;
    root_h = w->h;
    root_arena_w = root_w;
    root_arena_h = root_h - menubar_h - statusbar_h - bar1_h - padding_h;
    widget_put_size(pattern_window, root_arena_w, root_arena_h);
    widget_put_size(machine_window, root_arena_w, root_arena_h);
    widget_put_size(song_window, root_arena_w, root_arena_h);
    widget_put_size((widget_ptr) mb, root_w, menubar_h);
    song->x = root_arena_w;
    song->y = root_arena_h;
    widget_put_size((widget_ptr) statuslabel, root_w, statusbar_h);
    widget_put_local((widget_ptr) statuslabel, 0, root_h - statusbar_h);
}

void put_status_text(char *s)
{
    label_put_text(statuslabel, s);
}

static void win_file_op(int op)
{
    char *t;
    switch(op) {
	case op_open:
	    t = "Open";
	    break;
	case op_saveas:
	    t = "Save";
	    break;
	case op_import:
	    t = "Import";
	    break;
	default:
	    //bug if reached
	    return;
    }
    window_put_title(filewin, t);
    file_op = op;
    window_modal_open(filewin);
    widget_put_local((widget_ptr) filewin, 50, 50);
}

void perform_file_op(char *filename)
{
    switch (file_op) {
	case op_open:
	    audio_pause();
	    if (song_load(song, filename)) {
		put_status_text("Open failed");
	    } else {
		root_edit_song(song);
		put_status_text("Opened");
	    }
	    break;
	case op_saveas:
	    song_save(song, filename);
	    put_status_text("Saved");
	    break;
	case op_import:
	    audio_pause();
	    if (song_import_buzz(song, filename)) {
		put_status_text("Import failed");
	     } else {
		root_edit_song(song);
		put_status_text("Imported");
	     }
	    break;
	default:
	    //bug if reached
	    return;
    }
}

static void load_song_cb(widget_ptr w, void *data)
{
    win_file_op(op_open);
}

static void saveas_song_cb(widget_ptr w, void *data)
{
    win_file_op(op_saveas);
}

void root_new_song()
{
    song_init(song);
    song->master = song_create_machine_auto_id(song, "Master");
    song->x = root_arena_w;
    song->y = root_arena_h;
    machine_window_center((machine_window_ptr) machine_window, song->master);
    song_rewind(song);
    root_edit_song(song);
    put_status_text("New Song");
}

static void new_song_cb(widget_ptr w, void *data)
{
    song_clear(song);
    audio_pause();
    root_new_song();
}

static void import_buzz_cb(widget_ptr w, void *data)
{
    win_file_op(op_import);
}

static void quit_cb(widget_ptr w, void *data)
{
    main_quit();
}

static void show_area(widget_ptr w)
{
    if (now_showing) {
	widget_hide(now_showing);
    }
    now_showing = w;
    widget_show(w);
}

void show_pattern_window_cb(widget_ptr w, void *data)
{
    show_area(pattern_window);
}

void show_machine_window_cb(widget_ptr w, void *data)
{
    show_area(machine_window);
}

void show_song_window_cb(widget_ptr w, void *data)
{
    show_area(song_window);
}

static void add_area(widget_ptr w)
{
    window_put_widget(rootwin, w, 0, menubar_h + bar1_h + padding_h);
    widget_hide(w);
}

static void play_cb(widget_ptr w, void *data)
{
    audio_play();
    put_status_text("Playing");
}

static void rewind_cb(widget_ptr w, void *data)
{
    song_rewind(song);
}

static void pause_cb(widget_ptr w, void *data)
{
    audio_pause();
    put_status_text("Paused");
}

static void about_cb(widget_ptr caller, void *data)
{
    widget_ptr w = (widget_ptr) aboutwin;
    widget_put_local(w, 50, 50);
    widget_show(w);
}

static void cancelbpm_cb(widget_ptr caller, void *data)
{
    char buf[8];
    sprintf(buf, "%d", song->bpm);
    textbox_put_text(bpmtb, buf);
    widget_focus(NULL);
}

static void canceltpb_cb(widget_ptr caller, void *data)
{
    char buf[8];
    sprintf(buf, "%d", song->tpb);
    textbox_put_text(tpbtb, buf);
    widget_focus(NULL);
}

static void editbpm_cb(widget_ptr caller, void *data)
{
    char buf[8];
    cell_t c;
    sprintf(buf, "b%x", atoi(bpmtb->text));
    cell_init_string(c, buf);
    machine_parse(song->master, c, -1);
    cell_clear(c);
    cancelbpm_cb(caller, data);
}

static void edittpb_cb(widget_ptr caller, void *data)
{
    char buf[8];
    cell_t c;
    sprintf(buf, "t%x", atoi(tpbtb->text));
    cell_init_string(c, buf);
    machine_parse(song->master, c, -1);
    cell_clear(c);
    canceltpb_cb(caller, data);
}

static int handle_key(widget_ptr w, int key)
{
    int status = 1;

    switch(key) {
	case SDLK_F2:
	    show_area(pattern_window);
	    break;
	case SDLK_F3:
	    show_area(machine_window);
	    break;
	case SDLK_F4:
	    show_area(song_window);
	    break;
	case SDLK_F5:
	    play_cb(w, NULL);
	    break;
	case SDLK_F6:
	    rewind_cb(w, NULL);
	    break;
	case SDLK_F8:
	    pause_cb(w, NULL);
	    break;
	case SDLK_F12:
	    main_screenshot();
	    put_status_text("Screenshot Taken");
	    break;
	case SDLK_q:
	    if (widget_getmod((widget_ptr) rootwin) & KMOD_CTRL) {
		main_quit();
	    } else status = 0;
	    break;
	default:
	    status = 0;
	    break;
    }
    return status;
}

window_ptr root_new(int w, int h)
{
    menu_ptr m;
    menuitem_ptr ittop;
    menuitem_ptr it;
    label_ptr l;

    window_ptr win = rootwin;

    window_init(win);
    machine_window = (widget_ptr) machine_window_new();
    song_window = (widget_ptr) song_area_new();
    pattern_window = (widget_ptr) pattern_area_new();
    menubar_init(mb);
    label_init(statuslabel);

    window_put_widget(win, (widget_ptr) mb, 0, 0);

    add_area(machine_window);
    add_area(song_window);
    add_area(pattern_window);
    show_area(machine_window);

    window_put_widget(win, (widget_ptr) statuslabel, 0, 0);
    //will be relocated properly during resize

    //BPM, TPB textboxes
    l = label_new();
    label_put_text(l, "BPM:");
    label_shrinkwrap(l);
    window_put_widget(win, (widget_ptr) l, 10, menubar_h);

    textbox_init(bpmtb);
    widget_put_size((widget_ptr) bpmtb, 30, bar1_h - 2);
    window_put_widget(win, (widget_ptr) bpmtb, 45, menubar_h + 2);
    widget_connect((widget_ptr) bpmtb, signal_activate, editbpm_cb, NULL);
    widget_connect((widget_ptr) bpmtb, signal_cancel, cancelbpm_cb, NULL);

    l = label_new();
    label_put_text(l, "TPB:");
    label_shrinkwrap(l);
    window_put_widget(win, (widget_ptr) l, 100, menubar_h);

    textbox_init(tpbtb);
    widget_put_size((widget_ptr) tpbtb, 30, bar1_h - 2);
    window_put_widget(win, (widget_ptr) tpbtb, 135, menubar_h + 2);
    widget_connect((widget_ptr) tpbtb, signal_activate, edittpb_cb, NULL);
    widget_connect((widget_ptr) tpbtb, signal_cancel, canceltpb_cb, NULL);

    //File menu
    ittop = menuitem_new();
    menuitem_put_text(ittop, "File");
    menubar_add(mb, ittop);
    m = menu_new();
    menuitem_set_submenu(ittop, m);

    it = menuitem_new();
    menuitem_put_text(it, "New");
    menu_add(m, it);
    widget_connect((widget_ptr) it, signal_activate, new_song_cb, NULL);

    it = menuitem_new();
    menuitem_put_text(it, "Open...");
    menu_add(m, it);
    widget_connect((widget_ptr) it, signal_activate, load_song_cb, NULL);

    it = menuitem_new();
    menuitem_put_text(it, "Save As...");
    menu_add(m, it);
    widget_connect((widget_ptr) it, signal_activate, saveas_song_cb, NULL);

    it = menuitem_new();
    menuitem_put_text(it, "Import Buzz Song...");
    menu_add(m, it);
    widget_connect((widget_ptr) it, signal_activate, import_buzz_cb, NULL);

    it = menuitem_new();
    menuitem_put_text(it, "Quit");
    menu_add(m, it);
    widget_connect((widget_ptr) it, signal_activate, quit_cb, NULL);

    //Window menu
    ittop = menuitem_new();
    menuitem_put_text(ittop, "Window");
    menubar_add(mb, ittop);
    m = menu_new();
    menuitem_set_submenu(ittop, m);

    it = menuitem_new();
    menuitem_put_text(it, "Pattern");
    menu_add(m, it);
    widget_connect((widget_ptr) it, signal_activate, show_pattern_window_cb, NULL);
    it = menuitem_new();
    menuitem_put_text(it, "Machine");
    menu_add(m, it);
    widget_connect((widget_ptr) it, signal_activate, show_machine_window_cb, NULL);
    it = menuitem_new();
    menuitem_put_text(it, "Song");
    menu_add(m, it);
    widget_connect((widget_ptr) it, signal_activate, show_song_window_cb, NULL);

    //Action menu
    ittop = menuitem_new();
    menuitem_put_text(ittop, "Action");
    menubar_add(mb, ittop);
    m = menu_new();
    menuitem_set_submenu(ittop, m);

    it = menuitem_new();
    menuitem_put_text(it, "Play");
    menu_add(m, it);
    widget_connect((widget_ptr) it, signal_activate, play_cb, NULL);
    it = menuitem_new();
    menuitem_put_text(it, "Rewind");
    menu_add(m, it);
    widget_connect((widget_ptr) it, signal_activate, rewind_cb, NULL);
    it = menuitem_new();
    menuitem_put_text(it, "Pause");
    menu_add(m, it);
    widget_connect((widget_ptr) it, signal_activate, pause_cb, NULL);

    //Help menu
    ittop = menuitem_new();
    menuitem_put_text(ittop, "Help");
    menubar_add(mb, ittop);
    m = menu_new();
    menuitem_set_submenu(ittop, m);
    it = menuitem_new();
    menuitem_put_text(it, "About");
    menu_add(m, it);
    widget_connect((widget_ptr) it, signal_activate, about_cb, NULL);

    win->handle_key = handle_key;
    window_set_style(win, 0, 0);
    widget_connect((widget_ptr) win, signal_resize, root_resize, NULL);

    aboutwin = about_new();
    widget_hide((widget_ptr) aboutwin);
    filewin = filewin_new();

    widget_put_size((widget_ptr) win, w, h);
    root_new_song();
    put_status_text("Welcome to Bliss");

    return win;
}
