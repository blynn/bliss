#include "machine.h"

void machine_info_init(machine_info_ptr mi)
{
    mi->type = machine_effect;
    mi->id = "No Effect";
    mi->name = "NOP";
}
