#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "pl.h"

void *pl_load(char *filename)
{
    HINSTANCE p;

    p = LoadLibrary(filename);

    return (void *) p;
}

void pl_clear(void *p)
{
    FreeLibrary((HINSTANCE) p);
}

void *pl_sym(void *p, char *sym)
{
    void *r;
    r = GetProcAddress((HINSTANCE) p, sym);
    return r;
}
