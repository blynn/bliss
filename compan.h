#ifndef COMPAN_H
#define COMPAN_H

#include "button.h"

void compan_pop(widget_ptr compan);
void compan_push(widget_ptr compan, darray_ptr button_menu);
void compan_put(widget_ptr compan, darray_ptr button_menu);
button_ptr compan_new_button(widget_ptr compan, int row, int col);
void compan_init(widget_ptr compan, widget_ptr parent);

#endif //COMPAN_H
