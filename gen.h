#ifndef GEN_H
#define GEN_H

#include <stdlib.h>
#include <string.h>
#include "darray.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif //M_PI

extern double samprate, inv_samprate;
extern double nyquist, inv_nyquist;

struct gen_s;
typedef struct gen_s *gen_ptr;

struct note_s;
struct gen_data_s {
    void *data;
    int alive;
    double output; //one output per note (per node)
    struct note_s *note; //TODO: remove and add more builtins?
};

typedef struct gen_data_s gen_data_t[1];
typedef struct gen_data_s *gen_data_ptr;

enum {
    param_double = 0,
    param_string,
    param_count,
};

struct param_s {
    char *id;
    int type;
    void (*callback)(gen_ptr, void *);
};

typedef struct param_s *param_ptr;
typedef struct param_s param_t[1];

struct gen_info_s {
    char *id;
    char *name;
    void (*init)(struct gen_s *);
    void (*clear)(struct gen_s *);
    void *(*note_on)();
    void (*note_free)(void *);
    double (*tick)(struct gen_s *, gen_data_ptr, double *);
    int port_count;
    char **port_name;
    int param_count;
    param_ptr *param;
};

typedef struct gen_info_s *gen_info_ptr;
typedef struct gen_info_s gen_info_t[1];

struct gen_s {
    gen_info_ptr info;
    void **param;
    void *data;
};
typedef struct gen_s gen_t[1];

#include "note.h"

void gen_init(gen_t g, gen_info_t gi);
gen_ptr gen_new(gen_info_t gi);
void *gen_note_on(gen_t g);
void gen_note_free(gen_t g, void *data);
double gen_tick(gen_t g, gen_data_ptr gd, double *value);
void gen_clear(gen_t g);
void gen_free(gen_ptr g);

static inline double double_clip(double d, double min, double max)
{
    if (d < min) return min;
    if (d > max) return max;
    return d;
}

static inline double to_double(void *data)
{
    return *((double *) data);
}

static inline void from_double(void *data, double d)
{
    *((double *) data) = d;
}

static inline void assign_double(gen_ptr g, int i, double d)
{
    void *data = g->param[i];
    from_double(data, d);
    g->info->param[i]->callback(g, data);
}

static inline void from_string(void **data, char *s)
{
    free(*data);
    *data = malloc(strlen(s) + 1);
    strcpy(*data, s);
}

static inline void assign_string(gen_ptr g, int i, char *s)
{
    void *data = g->param[i];
    from_string(&data, s);
    g->info->param[i]->callback(g, data);
}

void param_type_init();

#endif //GEN_H
