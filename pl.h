#ifndef PL_H
#define PL_H

void* pl_load(char *filename);
void pl_clear(void* p);
void *pl_sym(void*, char*);

#endif //PL_H
