#ifndef GEN_H
#define GEN_H

#include "darray.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif //M_PI

extern double samprate, inv_samprate;
extern double nyquist, inv_nyquist;

struct gen_s;
typedef struct gen_s *gen_ptr;
typedef struct gen_s gen_t[1];

struct note_s;
struct gen_data_s {
    void *data;
    int alive;
    struct note_s *note; //TODO: remove and add more builtins?
};

typedef struct gen_data_s gen_data_t[1];
typedef struct gen_data_s *gen_data_ptr;

struct param_s {
    char *id;
    double init_val;
    void (*callback)(gen_ptr, double);
};

typedef struct param_s *param_ptr;

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
    double *param;
    void *data;
};

#include "note.h"

void gen_init(gen_t g, gen_info_t gi);
gen_ptr gen_new(gen_info_t gi);
void *gen_note_on(gen_t g);
void gen_note_free(gen_t g, void *data);
double gen_tick(gen_t g, gen_data_ptr gd, double *value);
void gen_clear(gen_t g);

static inline double double_clip(double d, double min, double max)
{
    if (d < min) return min;
    if (d > max) return max;
    return d;
}

#endif //GEN_H
