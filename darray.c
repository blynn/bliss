#include <stdlib.h>
#include <stdio.h>
#include "darray.h"

enum {
    max_init = 8
};

void darray_init(darray_ptr a)
{
    a->max = max_init;
    a->count = 0;
    a->item = malloc(sizeof(void *) * a->max);
}

void darray_remove_all(darray_ptr a)
{
    a->max = max_init;
    a->count = 0;
    free(a->item);
    a->item = malloc(sizeof(void *) * a->max);
}

void darray_realloc(darray_ptr a, int size)
{
    a->max = size;
    a->item = realloc(a->item, sizeof(void *) * a->max);
}

void darray_append(darray_ptr a, void *p)
{
    if (a->count == a->max) {
	if (!a->max) a->max = max_init;
	else a->max *= 2;
	a->item = realloc(a->item, sizeof(void *) * a->max);
    }
    a->item[a->count] = p;
    a->count++;
}

int darray_index_of(darray_ptr a, void *p)
{
    int i;
    for (i=0; i<a->count; i++) {
	if (a->item[i] == p) return i;
    }
    return i;
}

void darray_clear(darray_t a)
{
    free(a->item);
    a->max = 0;
    a->count = 0;
}

void *darray_at(darray_t a, int i)
{
    return a->item[i];
}

void darray_show(darray_ptr a)
{
    int i;
    for (i=0;i<a->count;i++) {
	printf("%d: %X\n", i, (int) a->item[i]);
    }
}

void darray_remove(darray_ptr a, void *p)
{
    int i;
    for (i=0; i<a->count; i++) {
	if (a->item[i] == p) {
	    for (;i<a->count; i++) {
		a->item[i] = a->item[i+1];
	    }
	}
    }
    a->count--;
}

void darray_copy(darray_ptr dst, darray_ptr src)
{
    int i;
    darray_realloc(dst, src->count);
    //TODO: bcopy instead
    for (i=0; i<src->count; i++) {
	dst->item[i] = src->item[i];
    }
    dst->count = src->count;
}

int darray_is_empty(darray_ptr d)
{
    return !d->count;
}
