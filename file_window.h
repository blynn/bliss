#ifndef FILE_WINDOW_H
#define FILE_WINDOW_H

#include "window.h"
#include "label.h"
#include "textbox.h"
#include "button.h"

struct file_window_s {
    window_t win;
    button_t bok, bcancel;
    textbox_t tbfilename;
    void (*callback)(char *);
};
typedef struct file_window_s file_window_t[1];
typedef struct file_window_s *file_window_ptr;

void file_window_setup(file_window_ptr fw, char *title, char *buttontext,
	void (*callback)(char *));
void file_window_init(file_window_ptr win, widget_ptr parent);

#endif //FILE_WINDOW_H
