#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "util.h"

char *strclone(char *s)
{
    char *r;

    r = (char *) malloc(strlen(s) + 1);
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

int strtonote(char *s)
{
    int n;
    if (!strcmp(s, "off")) {
	return 255;
    }
    n = notechar_to_int(*s);
    s++; if (!*s) return 255;
    if (*s == '#') {
	n++;
	s++; if (!*s) return 255;
    }
    n = n + 12 * (*s - '0');
    if (n >= 127) return 255;
    if (n < 0) return 255;
    return n;
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

char *note_to_text(int note)
{
    static int first = 1;
    static char *table[127];
    if (first) {
	char buf[8];
	int i, j;
	char c = 'C';
	int octave = 0;
	for (i=0; i<127; i++) {
	    buf[0] = c;
	    j = 1;
	    switch (i % 12) {
		case 1:
		case 3:
		case 6:
		case 8:
		case 10:
		    buf[j++] = '#';
		    c++;
		    if (c > 'G') c = 'A';
		    break;
		case 4:
		case 11:
		    c++;
		    if (c > 'G') c = 'A';
		default:
		    break;
	    }
	    sprintf(&buf[j], "%X", octave);
	    table[i] = strclone(buf);
	    if (i % 12 == 11) octave++;
	}
	first = 0;
    }

    if (note >= 127) return "off";
    return table[note];
}

int hex_to_int(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    return (c | 32) - 'a' + 10;
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
