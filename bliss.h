#include "orch.h"

extern orch_t orch;

void play_state_init();
int play_state_finished();
int play_state_delta();
void play_state_rewind();
void play_state_advance(void (*handler)(ins_ptr, event_ptr));
