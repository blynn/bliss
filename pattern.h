#ifndef PATTERN_H
#define PATTERN_H

#include <stdio.h>
#include "cell.h"

struct machine_s;

struct rowlist_s;

struct pattern_s {
    char *id;
    struct machine_s *machine;
    struct rowlist_s *first; //sentinel
    struct rowlist_s *last;
};

typedef struct pattern_s *pattern_ptr;
typedef struct pattern_s pattern_t[1];

void pattern_init(pattern_ptr p, struct machine_s* m);
pattern_ptr pattern_new(struct machine_s* m);
void pattern_clear(pattern_ptr p);
void pattern_tick(pattern_ptr p, int tick);
void pattern_put(pattern_ptr p, char *text, int x, int y);
char *pattern_at(pattern_ptr p, int x, int y);
void pattern_remove(pattern_ptr p, int x, int y);
void pattern_print(pattern_ptr p, FILE *fp);
void pattern_insert(pattern_ptr p, int x, int y);
void pattern_delete(pattern_ptr p, int x, int y);
#endif //PATTERN_H
