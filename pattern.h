#ifndef PATTERN_H
#define PATTERN_H

struct machine_s;

struct cell_s;

typedef struct cell_s *cell_ptr;
typedef struct cell_s cell_t[1];

struct cell_s {
    int x, y;
    char *text;
    cell_ptr next;
};

struct pattern_s {
    char *id;
    struct machine_s *machine;
    cell_t first;
};

typedef struct pattern_s *pattern_ptr;
typedef struct pattern_s pattern_t[1];

void pattern_init(pattern_ptr p, struct machine_s* m);
pattern_ptr pattern_new(struct machine_s* m);
void pattern_clear(pattern_ptr p);
void pattern_tick(pattern_ptr p, int tick);
void pattern_put(pattern_ptr p, char *text, int x, int y);
char *pattern_at(pattern_ptr p, int x, int y);
void pattern_delete(pattern_ptr p, int x, int y);
#endif //PATTERN_H
