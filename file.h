#ifndef FILE_H
#define FILE_H

#include "orch.h"

void file_init(); //call before using the following
void file_load(char *filename, orch_ptr orch);
void file_save(char *filename, orch_ptr orch);

#endif //FILE_H
