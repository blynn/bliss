#include "buzz_machine.h"

void buzz_machine_init()
{
    add_global_param(pt_word, "Unknown", "Unknown", 0, 0, 0, 0, 0);
    add_track_param(pt_note, "Note", "Note", NOTE_MIN, NOTE_MAX, NOTE_NO, 0, 0);
    add_track_param(pt_byte, "Sample", "Sample Number", 1, 128, 0, 0, 0);
    add_track_param(pt_byte, "Volume", "Volume Level", 0, 254, 255, 0, 255);
    add_track_param(pt_byte, "Effect", "Effect Number", 1, 255, 0, 0, 0);
    add_track_param(pt_byte, "Data", "Effect Data", 0, 255, 0, 0, 0);
}
