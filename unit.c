#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "unit.h"
#include "util.h"

void unit_init(unit_t u, unit_info_ptr ui, char *id)
{
    u->type = ut_plugin;
    u->ui = ui;
    u->id = strclone(id);
    darray_init(u->out);
    darray_init(u->in);
}

void unit_clear(unit_t u)
{
    free(u->id);
    darray_clear(u->out);
    darray_clear(u->in);
}

unit_ptr unit_new(unit_info_ptr ui, char *id)
{
    unit_ptr u = (unit_ptr) malloc(sizeof(struct unit_s));
    unit_init(u, ui, id);
    return u;
}
