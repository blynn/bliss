#ifndef UTABLE_H
#define UTABLE_H

#include "widget.h"
#include "graph.h"
#include "gen.h"

struct uentry_s {
    char *namebase;
    gen_info_ptr info;
    image_ptr img;
    void (*draw_control)(widget_ptr w, node_ptr node);
};
typedef struct uentry_s uentry_t[1];
typedef struct uentry_s *uentry_ptr;

void utable_init();
void utable_clear();
uentry_ptr utable_at(char *id);
extern uentry_ptr out_uentry;

#endif //UTABLE_H
