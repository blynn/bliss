#ifndef DARRAY_H
#define DARRAY_H

struct darray_s {
    void **item;
    int count;
    int max;
};

typedef struct darray_s darray_t[1];
typedef struct darray_s *darray_ptr;

void darray_init(darray_t a);
void darray_append(darray_t a, void *p);
void *darray_at(darray_t a, int i);
void darray_clear(darray_t a);
int darray_index_of(darray_ptr a, void *p);
void darray_remove(darray_ptr a, void *p);
void darray_copy(darray_ptr dst, darray_ptr src);
int darray_is_empty(darray_ptr d);
void darray_remove_all(darray_ptr d);

#endif //DARRAY_H
