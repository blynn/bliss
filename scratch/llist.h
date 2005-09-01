#include "llist.h"

void llist_init(llist_t l)
{
    l->first = NULL;
    l->last = NULL;
}

llist_ptr llist_new()
{
    llist_ptr res;
    res = (llist_ptr) malloc(sizeof(llist_t));
    llist_init(res);
    return res;
}

void llist_append(llist_t l, void *p)
{
    llist_item_ptr item = (llist_item_ptr) malloc(sizeof(llist_item_t));
    item->data = p;
    if (l->last) {
	l->first = l->last = p;
    } else {
	item->next = NULL;
	item->prev = l->last;
	l->last->next= p;
    }
}
