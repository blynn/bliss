#ifndef MACHINE_H
#define MACHINE_H

#include "version.h"
#include "darray.h"
#include "pattern.h"

#include "audio.h" //TODO: get rid of this

enum {
    machine_out = 1,
    machine_in = 2,
    machine_generator = machine_out,
    machine_master = machine_in,
    machine_effect = machine_in | machine_out,
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
};

typedef struct machine_s *machine_ptr;
typedef struct machine_s machine_t[1];

#include "track.h"

struct edge_s {
    machine_ptr src, dst;
};

struct machine_info_s {
    //plugin must provide this information:
    int type;
    char *id;
    char *name;
    void (* init)(machine_ptr);
    void (* clear)(machine_ptr);
    void (* work)(struct machine_s *, double *, double *);
    void (* parse)(machine_t, char *cmd, int col);
    void (* tick)(machine_t);
    //this stuff gets filled in later:
    char *plugin;
    void *dlptr;
};

void machine_init(machine_t m, machine_info_t mi, char *id);
void machine_clear(machine_t m);
void machine_parse(machine_t m, char *cmd, int col);
void machine_tick(machine_t m);
machine_ptr machine_new(machine_info_t mi, char *id);
edge_ptr edge_new(machine_t src, machine_t dst);
void edge_clear(edge_ptr);

void machine_next_sample(machine_ptr m, double *l, double *r);
pattern_ptr machine_create_pattern_auto_id(machine_ptr m);
pattern_ptr machine_create_pattern(machine_ptr m, char *id);

#endif //MACHINE_H
