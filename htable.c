#include <stdlib.h>
#include "htable.h"

void htable_init(htable_ptr h)
{
    darray_init(h->d);
    darray_init(h->k);
}

htable_ptr htable_new()
{
    htable_ptr res;
    res = (void *) malloc(sizeof(htable_t));
    htable_init(res);
    return res;
}

void htable_clear(htable_ptr h)
{
    darray_clear(h->d);
    darray_clear(h->k);
}

void htable_free(htable_ptr h)
{
    htable_clear(h);
    free(h);
}

void htable_remove_all(htable_ptr h)
{
    darray_remove_all(h->d);
    darray_remove_all(h->k);
}

void htable_put(htable_ptr h, void *data, void *key)
{
    darray_append(h->d, data);
    darray_append(h->k, key);
}

int htable_has(htable_ptr h, void *key)
{
    int i;
    for (i=0; i<h->k->count; i++) {
	if (key == h->k->item[i]) return -1;
    }
    return 0;
}

void *htable_at(htable_ptr h, void *key)
{
    int i;
    for (i=0; i<h->k->count; i++) {
	if (key == h->k->item[i]) return h->d->item[i];
    }
    return NULL;
}

void htable_remove(htable_ptr h, void *key)
{
    int i;
    for (i=0; i<h->k->count; i++) {
	if (key == h->k->item[i]) {
	    darray_remove_index(h->k, i);
	    darray_remove_index(h->d, i);
	}
    }

}

void htable_forall(htable_ptr h, void (*func)(void *))
{
    darray_forall(h->d, func);
}
