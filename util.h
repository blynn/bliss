#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <string.h>

static inline char *strclone(char *s)
{
    if (s) {
	char *res = (char *) malloc(strlen(s) + 1);
	strcpy(res, s);
	return res;
    } else return NULL;
}

#endif //UTIL_H
