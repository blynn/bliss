#include <stdlib.h>
#include "util.h"
#include "cell.h"

void cell_init_int(cell_ptr c, int i)
{
    c->type = t_int;
    c->data.i = i;
}

void cell_init_note(cell_ptr c, int i)
{
    c->type = t_note;
    c->data.i = i;
}

void cell_init_string(cell_ptr c, char *text)
{
    c->type = t_string;
    c->data.s = strclone(text);
}

void cell_clear(cell_ptr c)
{
    if (c->type == t_string) {
	free(c->data.s);
    }
}

char *cell_to_text(cell_ptr c)
{
    static char buf[30];

    switch (c->type) {
	case t_string:
	    return(c->data.s);
	case t_note:
	    return note_to_text(c->data.i);
	case t_int:
	    sprintf(buf, "%02X", c->data.i);
	    return buf;
	default:
	    printf("unhandled cell type\n");
	    exit(1);
	    break;
    }
}
