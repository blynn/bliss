#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

char *strclone(char *s)
{
    char *r;

    r = malloc(strlen(s) + 1);
    strcpy(r, s);
    return r;
}

int sample_at(unsigned char *buf, int i)
{
    int r;
    //TODO: tidy this
    r = buf[2*i + 1] << 8;
    r |= buf[2*i];
    if (r >= 32768) r -= 65536;
    return r;
}

int notechar_to_int(char c)
{
    static int table[255];
    static int first = 1;
    if (first) {
	first = 0;
	table['C'] = 0;
	table['D'] = 2;
	table['E'] = 4;
	table['F'] = 5;
	table['G'] = 7;
	table['A'] = 9;
	table['B'] = 11;
	table['c'] = 0;
	table['d'] = 2;
	table['e'] = 4;
	table['f'] = 5;
	table['g'] = 7;
	table['a'] = 9;
	table['b'] = 11;
    }

    return table[(int) c];
}

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

int hex_to_int(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    return (c | 32) - 'a';
}

char shift_key(unsigned char ch)
{
    static char shifttable[256];
    static int first = 1;

    void add_shiftstring(char *s1, char *s2)
    {
	int i;

	for (i=0; i<strlen(s1); i++) {
	    shifttable[(int) s1[i]] = s2[i];
	}
    }

    if (first) {
	int c;

	for (c=0; c<256; c++) shifttable[c] = c;

	for (c='a'; c<='z'; c++) shifttable[c] = c - 32;

	add_shiftstring("1234567890-=", "!@#$%^&*()_+");
	add_shiftstring("[]\\;',./`", "{}|:\"<>?~");
    }

    return shifttable[ch];
}
