#include <math.h>
#include "note.h"

double note_to_freq(int n)
{
    static double n2ftable[127];
    static int first = 1;

    if (first) {
        int i;
        first = 0;
        for (i=0; i<127; i++) {
            n2ftable[i] = 220 * pow(2, ((double) (i - 57)) / 12.0);
        }
    }
    return n2ftable[n];
}
