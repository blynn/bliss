#include <stdlib.h>
#include "ins.h"

void ins_init(ins_ptr ins)
{
    darray_init(ins->voice);
}

ins_ptr ins_new()
{
    ins_ptr res = (ins_ptr) malloc(sizeof(ins_t));
    ins_init(res);
    return(res);
}

void ins_clear(ins_ptr ins)
{
    int i;
    for (i=0; i<ins->voice->count; i++) {
	voice_free((voice_ptr) ins->voice->item[i]);
    }
    darray_clear(ins->voice);
}

voice_ptr ins_add_voice(ins_ptr ins, char *id)
{
    voice_ptr v = voice_new(id);
    darray_append(ins->voice, v);
    return v;
}

void ins_note_on(ins_ptr ins, int noteno, double volume)
{
    int i;
    for (i=0; i<ins->voice->count; i++) {
	voice_ptr v = ins->voice->item[i];
	if (v->notemin <= noteno && v->notemax >= noteno) {
	    voice_note_on(v, noteno, volume);
	}
    }
}

void ins_note_off(ins_ptr ins, int noteno)
{
    int i;
    for (i=0; i<ins->voice->count; i++) {
	voice_ptr v = ins->voice->item[i];
	if (v->notemin <= noteno && v->notemax >= noteno) {
	    voice_note_off(v, noteno);
	}
    }
}

double ins_tick(ins_ptr ins)
{
    int i;
    double res = 0.0;
    for (i=0; i<ins->voice->count; i++) {
	voice_ptr v = ins->voice->item[i];
	res += voice_tick(v);
    }
    return res;
}
