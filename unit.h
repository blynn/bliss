#ifndef UNIT_H
#define UNIT_H

#include "darray.h"

enum {
    up_output = 1,
    up_input = 2
};

enum {
    ut_plugin = 0,
    ut_param,
    ut_stream,
};

struct port_desc_s {
    int type;
    char *id;
};
typedef struct port_desc_s *port_desc_ptr;
typedef struct port_desc_s port_desc_t[1];

struct unit_info_s {
    //plugin must provide this information:
    char *id;
    char *name;

    void (* init)();
    void (* clear)();

    int port_count;
    port_desc_ptr port_desc;
    void (* connect_port)(void *, int, double *);
    void (* run)(void *);
    void *(* note_new)();
    void (* note_free)(void *);

    //this stuff gets filled in later:
    char *plugin;
    void *dlptr;
};

typedef struct unit_info_s unit_info_t[1];
typedef struct unit_info_s *unit_info_ptr;

struct unit_s {
    char *id;
    int type;
    unit_info_ptr ui;
    int x, y;
    darray_t in;
    darray_t out;
    int index;
};

typedef struct unit_s *unit_ptr;
typedef struct unit_s unit_t[1];

struct unit_edge_s {
    unit_ptr src, dst;
    int srcport, dstport;
};

typedef struct unit_edge_s *unit_edge_ptr;

void unit_init(unit_t u, unit_info_ptr ui, char *id);
void unit_clear(unit_t u);
unit_ptr unit_new(unit_info_ptr ui, char *id);

#endif //UNIT_H
