//hash table
//for now it's a slow simple implementation
#ifndef HTABLE_H
#define HTABLE_H

#include "darray.h"

struct htable_s {
    darray_t d;
    darray_t k;
};
typedef struct htable_s htable_t[1];
typedef struct htable_s *htable_ptr;

void htable_init(htable_ptr h);
htable_ptr htable_new();
void htable_clear(htable_ptr h);
void htable_free(htable_ptr h);
void htable_remove_all(htable_ptr h);
void htable_remove(htable_ptr h, void *key);
void htable_put(htable_ptr h, void *data, void *key);
void *htable_at(htable_ptr h, void *key);
int htable_has(htable_ptr h, void *key);
void htable_forall(htable_ptr h, void (*func)(void *));

#endif //HTABLE_H
