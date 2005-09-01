#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "widget.h"

struct textbox_s;
typedef struct textbox_s *textbox_ptr;

struct textbox_s {
    widget_t w;
    char s[128 + 1];
    char savedcopy[128 + 1];
    int len;
    int cursor;
    void (*ok_cb)(void *data, char *);
    void *ok_cb_data;
    int active;
};
typedef struct textbox_s textbox_t[1];

void textbox_put_string(textbox_t tb, char *s);
void textbox_update(textbox_t tb);
void textbox_handlembdown(widget_ptr w, int button, int x, int y);
void textbox_put_ok_callback(textbox_t tb,
	void (*func)(void *data, char *), void *data);
void textbox_ok(textbox_t tb);
void textbox_clear(textbox_ptr l);
void textbox_init(textbox_t tb, widget_ptr parent);
textbox_ptr textbox_new(widget_ptr parent);

#endif //TEXTBOX_H
