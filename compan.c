#include "button.h"

//can only have one instance:
static darray_t buttonmenustack;

static void compan_update(widget_ptr compan)
{
    int i;
    darray_ptr l = darray_last(buttonmenustack);

    widget_raised_background(compan);
    for (i=0; i<l->count; i++) {
	button_ptr p = (button_ptr) l->item[i];
	widget_ptr w = p->w;
	widget_update(w);
    }
}

void compan_pop(widget_ptr compan)
{
    darray_remove_last(buttonmenustack);
    widget_update(compan);
    request_update(compan);
    motion_notify();
}

void compan_push(widget_ptr compan, darray_ptr button_menu)
{
    darray_append(buttonmenustack, button_menu);
    widget_update(compan);
    request_update(compan);
    motion_notify();
}

void compan_put(widget_ptr compan, darray_ptr button_menu)
{
    darray_remove_all(buttonmenustack);
    compan_push(compan, button_menu);
}

button_ptr compan_new_button(widget_ptr compan, int row, int col)
{
    button_ptr b;

    b = button_new(compan);

    widget_ptr w = b->w;
    w->localx = col * (32 + 4 + 4) + 8;
    w->localy = row * (32 + 4 + 4) + 8;
    w->w = 32 + 4;
    w->h = 32 + 4;

    return b;
}

static void compan_handle_mousebuttondown(widget_ptr w,
	int button, int x, int y)
{
    int i;
    darray_ptr l = darray_last(buttonmenustack);
    for (i=0; i<l->count; i++) {
	button_ptr p = (button_ptr) l->item[i];
	if (local_contains(p->w, x, y)) {
	    p->w->handle_mousebuttondown(p->w, button, x - p->w->localx, y - p->w->localy);
	    return;
	}
    }
}

static void compan_motion(widget_ptr compan,
	int x0, int y0, int x1, int y1,
	void *data)
{
    int i;
    darray_ptr l = darray_last(buttonmenustack);
    //TODO: only need to remove old selection box and text
    //(no need to redraw whole control panel)
    widget_update(compan);

    for (i=0; i<l->count; i++) {
	button_ptr p = (button_ptr) l->item[i];
	widget_ptr w = p->w;
	if (local_contains(w, x1, y1)) {
	    widget_rectangle(w, -1, -1,  w->w, w->h, c_select);
	    widget_string(compan, 10, compan->h - 12, p->text, c_text);
	    break;
	}
    }
    request_update(compan);
}

void compan_init(widget_ptr compan, widget_ptr parent)
{
    darray_init(buttonmenustack);
    widget_init(compan, parent);
    widget_show(compan);
    compan->update = compan_update;
    compan->handle_mousebuttondown = compan_handle_mousebuttondown;
    widget_bind_mouse_motion(compan, compan_motion, NULL);
}
