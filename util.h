#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>

static inline char *strclone(char *s)
{
    char *res = (char *) malloc(strlen(s) + 1);
    strcpy(res, s);
    return res;
}

#endif //UTIL_H
