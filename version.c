#include "version.h"

int bliss_version()
{
    return BLISS_VERSION;
}

char *bliss_version_string()
{
    static char *s = "0.01";
    return s;
}
