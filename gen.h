#ifndef GEN_H
#define GEN_H

#include "darray.h"

#ifndef M_PI
#define M_PI 3.14159265358979
#endif //M_PI

struct gen_s;
typedef struct gen_s *gen_ptr;
typedef struct gen_s gen_t[1];

struct param_s {
    char *id;
    double init_val;
    void (*callback)(gen_ptr, double);
};

typedef struct param_s *param_ptr;

struct gen_info_s {
    char *id;
    void (*init)(struct gen_s *);
    void (*clear)(struct gen_s *);
    void *(*note_on)();
    void (*note_free)(void *);
    double (*tick)(struct gen_s *, void *, double *);
    int port_count;
    char **port_name;
    int param_count;
    param_ptr *param;
};

typedef struct gen_info_s *gen_info_ptr;
typedef struct gen_info_s gen_info_t[1];

/*
struct note_off_info_s {
    void (*note_off)(void **);
};
typedef struct note_off_info_s note_off_info_t[1];
typedef struct note_off_info_s *note_off_info_ptr;
*/

struct gen_s {
    gen_info_ptr info;
    //note_off_info_ptr offp;
    double (*note_off_tick)(gen_ptr g, void *, double *, int *);
    double *param;
    void *data;
};

void gen_init(gen_t g, gen_info_t gi);
gen_ptr gen_new(gen_info_t gi);
void *gen_note_on(gen_t g);
void gen_note_free(gen_t g, void *data);
double gen_tick(gen_t g, void *data, double *value);
double gen_note_off_tick(gen_t g, void *data, double *value, int *dead);
void gen_clear(gen_t g);

#endif //GEN_H
