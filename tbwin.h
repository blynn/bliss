#ifndef TBWIN_H
#define TBWIN_H

#include "window.h"
#include "textbox.h"

struct tbwin_s {
    window_t win;
    textbox_t tb;
};

typedef struct tbwin_s *tbwin_ptr;
typedef struct tbwin_s tbwin_t[1];

void tbwin_init(tbwin_ptr t);
void tbwin_open(tbwin_ptr t);
void tbwin_put_title(tbwin_ptr t, char *title);

#endif //TBWIN_H
