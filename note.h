#ifndef NOTE_H
#define NOTE_H

struct ins_s;
struct gen_s;

struct note_data_s {
    struct gen_s *g;
    void *data;
    double output;
};

typedef struct note_data_s note_data_t[1];
typedef struct note_data_s *note_data_ptr;

struct note_s {
    double freq;
    double volume;
    struct ins_s *ins;
    int is_off;
    int ref_count;
    note_data_ptr *gen_data;
};

typedef struct note_s note_t[1];
typedef struct note_s *note_ptr;

double note_to_freq(int n);

#endif //NOTE_H
