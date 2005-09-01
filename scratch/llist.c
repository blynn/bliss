//A doubly linked-list implementation
#ifndef LLIST_H
#define LLIST_H

struct llist_item_s;
typedef struct llist_item_s llist_item_t[1];
typedef struct llist_item_s *llist_item_ptr;
struct llist_item_s {
    llist_item_ptr next, prev;
    void *data;
}

struct llist_s {
    llist_item_ptr first, last;
};

typedef struct llist_s llist_t[1];
typedef struct llist_s *llist_ptr;

struct llist_iterator_s {
    llist_ptr l;
    llist_item_ptr cursor;
};
typedef struct llist_iterator_s llist_iterator_t[1];
typedef struct llist_iterator_s *llist_iterator_ptr;

void llist_init(llist_t l);
llist_ptr llist_new();
void llist_append(llist_t l, void *p);
