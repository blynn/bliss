#ifndef MACHINE_H
#define MACHINE_H

#include <stdio.h>
#include "version.h"
#include "darray.h"
#include "pattern.h"
#include "cell.h"
#include "unit.h"

#include "audio.h" //TODO: get rid of this

enum {
    machine_out = 1,
    machine_in = 2,
    machine_generator = machine_out,
    machine_master = machine_in,
    machine_effect = machine_in | machine_out,
    machine_bliss = 32 | machine_effect,
};

struct edge_s;

typedef struct edge_s *edge_ptr;
typedef struct edge_s edge_t[1];

struct machine_info_s;

typedef struct machine_info_s *machine_info_ptr;
typedef struct machine_info_s machine_info_t[1];

//can't have circular dependencies
struct track_s;
struct song_s;

struct machine_s {
    char *id;
    void *data;
    int x, y;
    darray_t in;
    darray_t out;
    machine_info_ptr mi;
    int visited;
    double l, r;
    struct track_s *track;
    darray_t pattern;
    struct song_s *song;
    int *buzz_state;
};

typedef struct machine_s *machine_ptr;
typedef struct machine_s machine_t[1];

#include "track.h"
#include "song.h"

struct edge_s {
    machine_ptr src, dst;
    int srcport, dstport;
};

struct buzz_machine_info_s;

struct machine_info_s {
    //plugin must provide this information:
    int type;
    char *id;
    char *name;
    int is_bliss;
    void (* init)(machine_ptr);
    void (* clear)(machine_ptr);
    void (* work)(struct machine_s *, double *, double *);
    void (* parse)(machine_t, cell_t, int col);
    void (* tick)(machine_t);

    void (* cell_init)(cell_t c, machine_t, char *text, int col);
    void (* print_state)(machine_t, FILE *fp);
    struct buzz_machine_info_s* buzzmi;

    //this stuff gets filled in later:
    char *plugin;
    void *dlptr;

    //for b-machines:
    int btype;
    darray_t unit;
    darray_t unit_edge;
};

void machine_init(machine_t m, machine_info_t mi, struct song_s *s, char *id);
void machine_clear(machine_t m);
void machine_parse(machine_t m, cell_t, int col);
void machine_tick(machine_t m);
machine_ptr machine_new(machine_info_t mi, struct song_s *s, char *id);
edge_ptr edge_new(machine_t src, machine_t dst);
void edge_clear(edge_ptr);

void machine_next_sample(machine_ptr m, double *l, double *r);
pattern_ptr machine_create_pattern_auto_id(machine_ptr m);
pattern_ptr machine_create_pattern(machine_ptr m, char *id);
void machine_cell_init(cell_ptr c, machine_ptr m, char *text, int col);
void machine_print_state(machine_ptr m, FILE *fp);
pattern_ptr machine_pattern_at(machine_ptr m, char *id);
machine_info_ptr machine_info_new();

#endif //MACHINE_H
