#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include "pl.h"

void *pl_load(char *filename)
{
    void *p;
    char *s;

    p = dlopen(filename, RTLD_LAZY);

    if ((s = dlerror())) {
	fprintf(stderr, "error: '%s'\n", s);
	return NULL;
    }

    return p;
}

void pl_clear(void *p)
{
    dlclose(p);
}

void *pl_sym(void *p, char *sym)
{
    void *r;
    char *s;
    r = dlsym(p, sym);
    if ((s = dlerror())) {
	fprintf(stderr, "error: '%s'\n", s);
	return NULL;
    }
    return r;
}
