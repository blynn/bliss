#ifndef BUZZ_MACHINE_H
#define BUZZ_MACHINE_H

#include "darray.h"
#include "machine.h"

enum {
    pt_note = 0,
    pt_switch,
    pt_byte,
    pt_word,
    NOTE_MIN = 0,
    NOTE_MAX = 127,
    NOTE_NO = 0,
    MPF_STATE = 1,
    SWITCH_ON = 1,
    SWITCH_NO = 0,
};

struct buzz_param_s {
    int type;
    char *name;
    char *desc;
    int minval;
    int maxval;
    int noval;
    int flags;
    int defval;
    void (*func)(machine_ptr m, int track, int val);
};

typedef struct buzz_param_s *buzz_param_ptr;
typedef struct buzz_param_s buzz_param_t[1];

struct buzz_machine_info_s {
    int type;
    darray_t gparam;
    darray_t tparam;
    int gpsize;
    int tpsize;
    char *id;
    char *name;
    void (* init)(machine_ptr);
    void (* clear)(machine_ptr);
    void (* work)(struct machine_s *, double *, double *);
    void (* tick)(machine_t);
    int track_max;
};

typedef struct buzz_machine_info_s *buzz_machine_info_ptr;
typedef struct buzz_machine_info_s buzz_machine_info_t[1];

void buzz_param_init();
void buzz_machine_info_init(buzz_machine_info_ptr mi);
void global_param(buzz_param_ptr bp);
void track_param(buzz_param_ptr bp);

#endif //BUZZ_MACHINE_H
