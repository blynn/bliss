#ifndef CELL_H
#define CELL_H

enum {
    t_int,
    t_string,
    t_note,
};

struct cell_s {
    int type;
    union {
	int i;
	char *s;
    } data;
};

typedef struct cell_s cell_t[1];
typedef struct cell_s* cell_ptr;

void cell_clear(cell_ptr c);
void cell_init_string(cell_ptr c, char *text);
char *cell_to_text(cell_ptr c);
void cell_init_int(cell_ptr c, int i);
void cell_init_note(cell_ptr c, int i);

#endif //CELL_H