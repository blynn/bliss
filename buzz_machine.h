#ifndef BUZZ_MACHINE_H
#define BUZZ_MACHINE_H

#include "darray.h"

enum {
    pt_switch = 0,
    pt_byte,
    pt_word,
    pt_note,
    NOTE_MIN = 0,
    NOTE_MAX = 127,
    NOTE_NO = 0,
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
    void (*func)(int track, int val);
};

typedef struct buzz_param_s *buzz_param_ptr;
typedef struct buzz_param_s buzz_param_t[1];

struct buzz_machine_info_s {
    darray_t gparam;
    darray_t tparam;
};

typedef struct buzz_machine_info_s *buzz_machine_info_ptr;
typedef struct buzz_machine_info_s buzz_machine_info_t[1];

#endif //BUZZ_MACHINE_H
